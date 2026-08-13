pipeline {
    agent any

    environment {
        IMAGE_NAME = "TCP"
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        stage('Build Docker Image') {
            steps {
                sh 'docker build -t ${TCP_container} .'
            }
        }

        stage('Compile & Run') {
    steps {
        timeout(time: 30, unit: 'SECONDS') {
            sh 'docker run --rm ${TCP_container} sh -c "gcc TCP.c -o TCP -pthread && ./TCP"'
        }
    }
}
        }
    }

    post {
        always {
            sh 'docker image prune -f' // avoid disk buildup from repeated builds
        }
        failure {
            echo 'Build failed — check console output above.'
        }
    }
}
