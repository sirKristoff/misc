#!/usr/bin/python

# ##############################################################################
# MowerApp UnitTest Script
# ##############################################################################

tifTestRunner.StopOnFail = True


################################################################################
# USABLE TIFBASE COMMANDS
################################################################################

# Globally available tif handles
# ---------------------------------------------------------------------------- #
# tifDialogs
# tifTestRunner
# tifConsole
# tifDevice
# tifAsserts

# Good for debugging ect 
# ---------------------------------------------------------------------------- #
# tifConsole.Log("Your message here") # this writes a message to the console
# tifConsole.Log("Your message here: %s" %(awesomeVariable)) # this writes a message to the console with an variable
# tifConsole.LogError("Your message here") # this writes a errormessage to the console


# This is how to sent mowercommands to the mower
# ---------------------------------------------------------------------------- #
# tifDevice.Send("yourIRSCommand.here()", 300, True) # Sends a message to the mower
# tifDevice.SerialIo.Init("COM1") # Changes comport for tifapp 
# tifDevice.LoginAdvanced() #loginadvanced to mower


# Theese starts a loop for the targetedMethod and will run as long as the return value is False and progress is less than 100
# ---------------------------------------------------------------------------- #
# tifDialogs.ShowConditionalInstructionWithCallbackAndProgress("Title", "Description", self.targetedMethod, self.progress)
# tifDialogs.ShowConditionalInstructionWithCallback("Title", "Description", self.targetedMethod)



# To have pass fail without an assert.json file use theese
# (!) Description seems to not be printed anywhere, but Name is used in printouts and xml result file.
# ---------------------------------------------------------------------------- #
# tifTestRunner.AreEqual(True, boolean, "Description", "Name") # AreEqual compares boolean/int/string/double
# tifTestRunner.AreNotEqual(True, boolean, "Description", "Name") # AreNotEqual compares boolean/int/string/double
# tifTestRunner.AreGreaterThan(int, int, "Description", "Name") # AreGreaterThan compares int/double
# tifTestRunner.AreLessThan(int, int, "Description", "Name") # AreLessThan compares int/double
# tifTestRunner.AreInRange(intValue, intMin, intMax, "Description", "Name") # AreInRange compares range on int/double
# tifTestRunner.AreNotInRange(intValue, intMin, intMax, "Description", "Name") # AreNotInRange compares range on int/double

# tifTestRunner.Pass("Description") # Just a pass
# tifTestRunner.Fail("Description") # Just a fail



################################################################################
# Internal (private) functions
################################################################################
    
 
################################################################################
# Test cases
################################################################################

def testMinBinaryHeap( *args, **kwargs ):
    
    response = tifDevice.Send("TestBinaryHeap.TestMinHeap()", 5000, True)

    if response.ResponseResult != ResponseResult.Ok:
        tifTestRunner.Fail("TestBinaryHeap.TestMinHeap() response NOT OK!")

    if "result" not in response.OutParameters:
        tifTestRunner.Fail("Expected parameter 'result' missing!")
    
    result = response["result"]
    tifConsole.Log("result: {}".format(result))

    tifTestRunner.AreEqual(True, result, "TestBinaryHeap.TestMinHeap", "TestBinaryHeap.TestMinHeap")


def testMaxBinaryHeap( *args, **kwargs ):
    
    response = tifDevice.Send("TestBinaryHeap.TestMaxHeap()", 5000, True)

    if response.ResponseResult != ResponseResult.Ok:
        tifTestRunner.Fail("TestBinaryHeap.TestMaxHeap() response NOT OK!")

    if "result" not in response.OutParameters:
        tifTestRunner.Fail("Expected parameter 'result' missing!")
    
    result = response["result"]
    tifConsole.Log("result: {}".format(result))

    tifTestRunner.AreEqual(True, result, "TestBinaryHeap.TestMaxHeap", "TestBinaryHeap.TestMaxHeap")
