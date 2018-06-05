pipeline {

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
