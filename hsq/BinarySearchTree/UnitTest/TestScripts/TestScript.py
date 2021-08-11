#!/usr/bin/python

###############################################################################
# BinarySearchTree UnitTest Script
###############################################################################

import time

TIF_TIMEOUT = 5000


################################################################################
# Internal (private) functions
################################################################################

def areEqual( actual, expected, description ):
    tifTestRunner.AreEqual( actual, expected, description, description )


def verifyResponseParamValue( response, expectedParam, parameter, description ):
    if response.ResponseResult == ResponseResult.Ok:
        if parameter not in response.OutParameters:
            tifTestRunner.Fail( "{} is not an out parameter".format( parameter ) )
        else:
            areEqual( expectedParam, response[parameter], description )
    else:
    	tifTestRunner.Fail( "Response not OK" )
    
 
################################################################################
# Test cases
################################################################################

# Test insertion into BST
def Insert( *args, **kwargs ):
    response = tifDevice.Send("TestBinarySearchTree.Insert()", TIF_TIMEOUT, False )
    verifyResponseParamValue( response, True, 'result', 'TestBinarySearchTree.Insert' )
   
    
# Test removal from BST
def Remove( *args, **kwargs ):
    response = tifDevice.Send("TestBinarySearchTree.Remove()", TIF_TIMEOUT, False )
    verifyResponseParamValue( response, True, 'result', 'TestBinarySearchTree.Remove' )


# Test removal of root node from BST
def RemoveRoot( *args, **kwargs ):
    response = tifDevice.Send("TestBinarySearchTree.RemoveRoot()", TIF_TIMEOUT, False )
    verifyResponseParamValue( response, True, 'result', 'TestBinarySearchTree.RemoveRoot' )
    

# Test search in BST
def Search( *args, **kwargs ):
    response = tifDevice.Send("TestBinarySearchTree.Search()", TIF_TIMEOUT, False )
    verifyResponseParamValue( response, True, 'result', 'TestBinarySearchTree.Search' )
    
    
# Test getting next (in-order successor) in BST
def Next( *args, **kwargs ):
    response = tifDevice.Send("TestBinarySearchTree.Next()", TIF_TIMEOUT, False )
    verifyResponseParamValue( response, True, 'result', 'TestBinarySearchTree.Next' )
    
    
# Test getting previous (in-order predecessor) in BST
def Previous( *args, **kwargs ):
    response = tifDevice.Send("TestBinarySearchTree.Previous()", TIF_TIMEOUT, False )
    verifyResponseParamValue( response, True, 'result', 'TestBinarySearchTree.Previous' )
	
	