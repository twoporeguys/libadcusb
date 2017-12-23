DOCS_PATH = '/mnt/builds/docs/libadcusb'

pipeline {
    agent { label 'master' }

    environment {
        CC = 'clang'
        CXX = 'clang++'
    }

    stages {
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
                sh "cp -a build/docs/* ${DOCS_PATH}/"
            }
        }
    }
}
