#
# Copyright 2017 Two Pore Guys, Inc.
# All rights reserved.
#
# This is proprietary software.
#

import os
import re
import gi
import time
import threading
import librpc
gi.require_version('Gtk', '3.0')


from gi.repository import Gtk, GLib


RESPONSE_TEMPLATE = """<span bgcolor="green">Request:</span>
<small><tt>{0}</tt></small>
<span bgcolor="red">Response:</span>
<small><tt>{1}</tt></small>
"""


class Context(object):
    def __init__(self):
        self.rpipe, self.wpipe = os.pipe()
        self.bus = librpc.Bus()
        self.set_status = None
        self.write_log = None
        self.device = None
        self.connection = None
        self.reader_thread = None
        self.ping_thread = None

    def connection_error(self, reason):
        print(reason)

    def connect(self, node):
        self.device = node
        self.connection = librpc.Client()
        self.connection.connect('usb://{0}'.format(node.name), librpc.Object(self.wpipe, librpc.ObjectType.FD))
        self.connection.error_handler = self.connection_error
        self.reader_thread = threading.Thread(target=self.log_reader, daemon=True)
        self.reader_thread.start()
        self.ping_thread = threading.Thread(target=self.pinger, daemon=True)
        self.ping_thread.start()

    def process_command(self, command):
        tokens = re.split(r'(?<!\\) ', command)
        tokens = [t.replace('\ ', ' ') for t in tokens]
        args = [eval(s) for s in tokens[1:]]

        if self.connection is None:
            raise RuntimeError("Not connected")

        return self.connection.call_sync(tokens[0], *args)

    def log_reader(self):
        f = os.fdopen(self.rpipe)
        while True:
            self.write_log(f.readline())

    def pinger(self):
        while True:
            node = self.device
            if not node:
                continue

            try:
                self.bus.ping(node.name)
                self.set_status('Connected to <b>{0}</b>'.format(node.name))
            except:
                self.set_status('Cannot connect to <b>{0}</b>'.format(node.name))

            time.sleep(1)


class DeviceSelectionWindow(Gtk.Dialog):
    def __init__(self, parent):
        super(DeviceSelectionWindow, self).__init__(
            "Select a device", parent, Gtk.DialogFlags.MODAL,
            (
                Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                Gtk.STOCK_OK, Gtk.ResponseType.OK
            )
        )

        self.context = parent.context
        self.store = Gtk.ListStore(str, str, object)
        self.tree = Gtk.TreeView(self.store)
        self.make_treeview()
        self.get_content_area().pack_start(self.tree, True, True, 0)
        self.set_default_size(300, 300)
        self.show_all()

    def make_treeview(self):
        column = Gtk.TreeViewColumn("Name")
        label = Gtk.CellRendererText()
        column.pack_start(label, True)
        column.add_attribute(label, "text", 0)
        self.tree.append_column(column)

        column = Gtk.TreeViewColumn("Serial number")
        label = Gtk.CellRendererText()
        column.pack_start(label, True)
        column.add_attribute(label, "text", 1)
        self.tree.append_column(column)

        for i in self.context.bus.enumerate():
            self.store.append((i.name, i.serial, i))

    @property
    def selected(self):
        _, treeiter = self.tree.get_selection().get_selected_rows()
        return self.store[treeiter][2]


class RPCPane(Gtk.Box):
    def __init__(self, parent):
        super(RPCPane, self).__init__()
        self.context = parent.context
        self.layout = Gtk.VBox()
        self.top = Gtk.HBox()
        self.input = Gtk.Entry()
        self.input.connect('activate', self.on_send)
        self.send = Gtk.Button("Send")
        self.viewport = Gtk.ScrolledWindow()
        self.viewport.set_hexpand(True)
        self.viewport.set_vexpand(True)
        self.log = Gtk.TextView()
        self.log.set_editable(False)
        self.viewport.add(self.log)
        self.top.pack_start(self.input, True, True, 0)
        self.top.pack_start(self.send, False, False, 0)
        self.layout.pack_start(self.top, False, False, 0)
        self.layout.pack_start(self.viewport, True, True, 0)
        self.add(self.layout)
        self.send.connect('clicked', self.on_send)

    def on_send(self, sender):
        line = self.input.get_text()
        buffer = self.log.get_buffer()
        end = buffer.get_end_iter()
        result = GLib.markup_escape_text(str(self.context.process_command(line)))
        buffer.insert_markup(end, RESPONSE_TEMPLATE.format(line, result), -1)
        self.log.scroll_to_iter(buffer.get_end_iter(), 0, False, 0, 0)
        self.input.set_text('')


class LogPane(Gtk.Box):
    def __init__(self, parent):
        super(LogPane, self).__init__()
        self.context = parent.context
        self.viewport = Gtk.ScrolledWindow()
        self.viewport.set_hexpand(True)
        self.viewport.set_vexpand(True)
        self.log = Gtk.TextView()
        self.log.set_editable(False)
        self.viewport.add(self.log)
        self.add(self.viewport)
        parent.context.write_log = self.write_log

    def write_log(self, log):
        buffer = self.log.get_buffer()
        end = buffer.get_end_iter()
        buffer.insert_markup(end, '<tt>{0}</tt>'.format(GLib.markup_escape_text(log)), -1)
        self.log.scroll_to_iter(buffer.get_end_iter(), 0, False, 0, 0)


class RecordPane(Gtk.Box):
    pass


class MainWindow(Gtk.Window):
    def __init__(self, ctx):
        super(MainWindow, self).__init__(title='MOM GUI')
        self.context = ctx
        self.context.set_status = self.set_status
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_size_request(640, 480)
        self.vbox = Gtk.VBox()
        self.banner = Gtk.Label("Not connected")
        self.context.status_label = self.banner
        self.notebook = Gtk.Notebook()
        self.notebook.append_page(RPCPane(self), Gtk.Label("RPC"))
        self.notebook.append_page(LogPane(self), Gtk.Label("Logs"))
        self.notebook.append_page(RecordPane(self), Gtk.Label("Data acquisition"))
        self.connect('delete-event', Gtk.main_quit)
        self.vbox.pack_start(self.banner, False, False, 5)
        self.vbox.pack_start(self.notebook, True, True, 0)
        self.add(self.vbox)

        GLib.idle_add(self.select_device)

    def select_device(self):
        dialog = DeviceSelectionWindow(self)
        resp = dialog.run()

        if resp == Gtk.ResponseType.CANCEL:
            Gtk.main_quit()

        self.context.connect(dialog.selected)
        dialog.destroy()

    def set_status(self, markup):
        self.banner.set_markup(markup)


if __name__ == '__main__':
    ctx = Context()
    win = MainWindow(ctx)
    win.show_all()
    Gtk.main()
