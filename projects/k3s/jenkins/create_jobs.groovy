import jenkins.model.*
import javaposse.jobdsl.plugin.*

def jobDslScript = '''
pipelineJob('GUCEF_Build') {
  definition {
    cps {
      script('''
        pipeline {
          agent any
          parameters {
            choice(name: 'BUILD_TYPE', choices: ['Debug', 'Release', 'RelWithDebInfo'], description: 'Choose the build type')
            choice(name: 'PLATFORM', choices: ['Linux', 'Windows', 'Android', 'AndroidTV', 'iPhone', 'MacOS', 'Arduino'], description: 'Choose the platform')
            choice(name: 'CPP_STANDARD', choices: ['C++98', 'C++20'], description: 'Choose the C++ standard')
            choice(name: 'TAGS', choices: ['services', 'tools', 'platform', 'plugins', 'dependencies'], description: 'Choose the tags')
          }
          environment {
            TARGETS_FILE = 'build_targets.json'
            DEPLOY_SERVER = 'your.server.com'
            DEPLOY_USER = 'your_user'
            DEPLOY_PATH = '/path/to/deploy'
          }
          stages {
            stage('Checkout') {
              steps {
                git 'https://github.com/VHarisop/GUCEF.git'
              }
            }
            stage('Determine Targets') {
              steps {
                script {
                  def targets = readJSON file: TARGETS_FILE
                  env.TARGETS = targets.join(',')
                }
              }
            }
            stage('Configure') {
              steps {
                script {
                  def cmakeCommand = "cmake -B build -DCMAKE_BUILD_TYPE=${params.BUILD_TYPE} -DCMAKE_CXX_STANDARD=${params.CPP_STANDARD} -G ${params.PLATFORM}"
                  if (isUnix()) {
                    sh cmakeCommand
                  } else {
                    bat cmakeCommand
                  }
                }
              }
            }
            stage('Build') {
              steps {
                script {
                  def buildCommand = "cmake --build build --target ${env.TARGETS}"
                  if (isUnix()) {
                    sh buildCommand
                  } else {
                    bat buildCommand
                  }
                }
              }
            }
          }
        }
      ''')
      sandbox()
    }
  }
}
'''

def jobDsl = new ExecuteDslScripts.ScriptRequest(
    jobDslScript, 
    true, 
    RemovedJobAction.DELETE, 
    RemovedViewAction.DELETE, 
    LookupStrategy.JENKINS_ROOT
)

def jobDslRunner = new ExecuteDslScripts()
jobDslRunner.perform(Jenkins.instance, [jobDsl])
