DOCS_PATH = '/var/www/docs/libadcusb'

pipeline {
    agent { label 'master' }

    stage('Build') {
        steps {
            sh 'mkdir -p build'
            sh 'cd build && cmake -DBUILD_DOC=ON ..'
            sh 'cd build && make'
        }
    }

    stage('Deploy docs') {
        steps {
            sh "rm -rf ${DOCS_PATH}"
            sh "mkdir -p ${DOCS_PATH}"
            sh "cp -a builds/docs/* ${DOCS_PATH}/"
        }
    }
}
