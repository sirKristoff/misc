#!/usr/bin/python

import sys
import git
import os
sys.path.append( '../../../../../../../BuildScripts' )

from rrBuildBaseVariants import rrSconsUnitTest

supportedTarget    = 'UnitTest-BinarySearchTree'
depFile            = 'UnitTest_BinarySearchTree/UnitTest_BinarySearchTree.py'

# ##########################################################################
## Returns root of GIT repository
#  @type  path  str
#  @param path  Absolute path to git ROOT
def getGitRoot():
    repo = git.Repo( '.', search_parent_directories = True )
    GIT_ROOT = repo.git.rev_parse( '--show-toplevel' )
    return GIT_ROOT

# ############################################################################
def findProjectPath():

    dir_path = os.path.dirname(os.path.realpath(__file__))
    dir_path = os.path.abspath( os.path.join( dir_path, os.path.pardir ) )
    gitRoot = getGitRoot()

    # Strip away begining of abs path so we have path relative from Git root. 
    rem = len(gitRoot)
    dir_path = dir_path[(rem + 1):]
    print ( f' path to project = {dir_path}')
    return dir_path        

# ############################################################################
def findStringForBuildRootDir():
    dir_path = os.path.dirname(os.path.realpath(__file__))

    buildRootDir = 'BuildScripts/Mower Main/'

    numberFolder = 0
    while not os.path.isdir( os.path.join( dir_path, buildRootDir ) ):
        dir_path = os.path.abspath( os.path.join( dir_path, os.path.pardir ) )
        numberFolder= numberFolder + 1

    for x in range( numberFolder ) :
        buildRootDir = os.path.join( '../', buildRootDir )

    print (buildRootDir)
    return buildRootDir

# ############################################################################
def main( buildName = '', dependencyFile = '', args = None ):

    projectPath = findProjectPath()
    buildRootDir = findStringForBuildRootDir()
    # We use a common class for Mower Main (see variant descriptions in the
    # imported file).
    itsBuilder = rrSconsUnitTest(
        supportedTarget    = supportedTarget,
        projectPath        = projectPath,
        dependencyFile     = depFile,
        buildName          = buildName,
        generateJsonVariant= 'StandardNode',
        buildRootDir       = buildRootDir,
        runTestScriptOnPort = 4350,
        args               = args
    )

    return( itsBuilder.doBuild() )

# ############################################################################
# To handle difference when importing and running standalone.
if ( __name__ == "__main__" ):
    x = main( supportedTarget, dependencyFile )
    if ( x != 0 ):
        print( 'ERROR - ' + supportedTarget + ' main() returned [' + str( x ) + ']' )
    sys.exit( x )