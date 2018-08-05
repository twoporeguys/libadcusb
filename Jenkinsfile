pipeline {
    agent {
        docker {
            image 'ubuntu:18.04'
            args "-u root:sudo -v ${env.DOCS_PATH}:${env.DOCS_PATH}"
        }
    }

    environment {
        CC = 'clang'
        CXX = 'clang++'
        DEBIAN_FRONTEND = 'noninteractive'
        LANG = 'C'
    }

    stages {
        stage('Bootstrap') {
            steps {
                sh 'apt-get update'
                sh 'apt-get -y install build-essential'
                sh 'make bootstrap'
            }
        }

        stage('Build') {
            steps {
                sh 'mkdir -p build'
                sh 'cd build && cmake -DBUILD_DOC=ON ..'
                sh 'cd build && make'
            }
        }

        stage('Deploy docs') {
            when {
                expression { "${env.DOCS_PATH}" != "" }
            }
            steps {
                sh "mkdir -p ${DOCS_PATH}/libadcusb"
                sh "rm -rf ${DOCS_PATH}/libadcusb/*"
                sh "cp -a build/docs/* ${DOCS_PATH}/libadcusb/"
            }
        }
    }
}
