#!/usr/bin/python3

# Okay, so we have a folder structure:
# tests -+-> command_tests
#        +-> instruction_tests
#        `-> etc_tests
#
# {test_type}_tests -+-> {test_type}_test_{test_name}.cmd
#
# And for each of those, there exists a file:
# {test_type}_tests/expected/{test_type}_test{test_name}.log
#
# So let's turn that into variables.

from pathlib import Path
import os
import io
import subprocess
import argparse

argParser = argparse.ArgumentParser(
    description="Run all available rv32sim tests.")
argParser.add_argument(
    '--test', help="Run this specific test, e.g. instruction_test_addi", action='store', dest='specificTest')
argParser.add_argument('--all', help="Run all tests, even if one fails.",
                       action='store_true', dest='runAllTests')
argParser.add_argument('--silent', help="Don't print any test output.",
                       action='store_true', dest='beSilent')
argParser.add_argument(
    '--noisy', help="Display rv32sim logs even if tests pass.", action='store_true', dest='beNoisy')
argParser.add_argument('--verbose', help="Enable verbose logging on rv32sim.",
                       action='store_true', dest='beVerbose')
argParser.add_argument('--commands', help="Display the commands provided to rv32sim.",
                       action='store_true', dest='showCommands')
argParser.add_argument(
    '--stage2', help="Run only Stage 2 tests.", action='store_true', dest='stage2')
argParser.add_argument('--bytes', help="Display bytes of expected and received output.",
                       action='store_true', dest='showBytes')
arguments = argParser.parse_args()

if (arguments.beSilent and arguments.beNoisy):
    print("You've set --silent and --noisy at the same time. Pick one.")
    exit(1)

testRootDir = Path("./tests")
rvsimExe = Path("./rv32sim.exe")

if not testRootDir.is_dir():
    print("Can't find the test directory! Please place it at '" + testRootDir.name + "'") 
    exit(1)

if not rvsimExe.is_file():
    print("Can't find the rv32sim executable! Please palce it at '" + rvsimExe.name + "'")
    exit(1)

COLOUR_ANSI_RED = "\033[1;31m"
COLOUR_ANSI_GREEN = "\033[1;32m"
COLOUR_ANSI_YELLOW = "\033[1;33m"
COLOUR_ANSI_BLUE = "\033[1;34m"
COLOUR_ANSI_MAGENTA = "\033[1;35m"
COLOUR_ANSI_CYAN = "\033[1;36m"
COLOUR_ANSI_DEFAULT = "\033[39;49m"

numTestPasses = 0
numTestFailures = 0

firstTestFailure = True


def red(s):
    """Make the string `s` appear red if printed."""
    return COLOUR_ANSI_RED + s + COLOUR_ANSI_DEFAULT


def green(s):
    """Make the string `s` appear green if printed."""
    return COLOUR_ANSI_GREEN + s + COLOUR_ANSI_DEFAULT


def yellow(s):
    """Make the string `s` appear yellow if printed."""
    return COLOUR_ANSI_YELLOW + s + COLOUR_ANSI_DEFAULT


def blue(s):
    """Make the string `s` appear blue if printed."""
    return COLOUR_ANSI_BLUE + s + COLOUR_ANSI_DEFAULT


def magenta(s):
    """Make the string `s` appear magenta if printed."""
    return COLOUR_ANSI_MAGENTA + s + COLOUR_ANSI_DEFAULT


def recordTestResult(passed: bool,
                     testName: str,
                     testResult: subprocess.CompletedProcess,
                     expectedBytes: bytes,
                     rvsimInput: bytes = b''):
    """Inform the user that the given test had the given results. 

    In practice, this prints a descriptive [PASS] or [FAIL] line, and possibly 
    prints logs and such depending on Run_Test.py's command-line arguments."""
    global numTestPasses, numTestFailures, firstTestFailure

    if passed:
        prefix = green(" Pass ")
        numTestPasses += 1
    else:
        numTestFailures += 1
        if testResult.returncode == 0:
            prefix = red(" Fail ")
        elif testResult.returncode == -6:
            prefix = red("Assert")
        elif testResult.returncode == -127:
            prefix = magenta("SigSegv")
        else:
            prefix = yellow("Crash ")

    print("[" + prefix + "]: ", testName, flush=True)

    shouldDisplayLogs = \
        not passed and not arguments.beSilent and firstTestFailure

    if arguments.beNoisy:
        shouldDisplayLogs = True

    if firstTestFailure and not passed:
        firstTestFailure = False

    if shouldDisplayLogs:
        print(yellow("Received Stdout"), ":", sep="")
        print(testResult.stdout.decode())
        print(yellow("End"))
        print(green("Expected Stdout"), ":", sep="")
        print(expectedBytes.decode())
        print(green("End"))

        if arguments.showBytes:
            print(blue("Stdout Bytes"), ":", sep="")
            print(testResult.stdout)
            print(blue("End"))
            print(blue("Expected Stdout Bytes"), ":", sep="")
            print(expectedBytes)
            print(blue("End"))

        if arguments.showCommands:
            print(yellow("Commands"), ":", sep="")
            print(rvsimInput.decode())
            print(yellow("End"))

        if len(testResult.stderr) != 0:
            print(yellow("Stderr"), ":", sep="")
            print(testResult.stderr.decode())
            print(yellow("End"))
        else:
            print(red("Nothing was output to stderr."))

        if testResult.returncode != 0:
            print("Note: rv32sim returned", testResult.returncode)

    if not arguments.runAllTests and not passed:
        print(red("Quitting on first test failure."))
        exit(0)


# ---

testCategories = [x for x in testRootDir.iterdir() if x.is_dir()]

for testCategory in testCategories:
    assert(testCategory.is_dir())

    if (arguments.stage2 and testCategory.name != "stage_2_tests"):
        continue

    testFiles = [x for x in testCategory.iterdir() if x.name.endswith('.cmd')]

    for testFile in testFiles:

        testName = testFile.name.split(".cmd", 1)[0]

        if arguments.specificTest is not None \
           and testName != arguments.specificTest:
            continue

        testFileBytes = io.open(testFile, "rb").read()

        rvsimArgsList = [rvsimExe.resolve()]
        if arguments.beVerbose:
            rvsimArgsList.append('-v')

        if testCategory.name == 'stage_2_tests':
            rvsimArgsList.append('-s2')

        testResult = subprocess.run(
            rvsimArgsList,
            input=testFileBytes,
            capture_output=True,
            cwd=testCategory
        )

        testPassed = True

        if testResult.returncode != 0:
            # Maybe not always true, but it really *should* be. If it passes the
            # tests but doesn't return 0, that sounds like a bug to me.
            testPassed = False

        # We're in the category directory. It should be ./expected/$testName.log
        expectedFile = testCategory / "expected" / (testName + ".log")
        assert expectedFile.exists()

        expectedFileBytes = io.open(expectedFile, "rb").read()

        # Sanitised strings.
        sReceived = testResult.stdout.decode().strip().replace("\r\n", "\n")
        sExpected = expectedFileBytes.decode().strip().replace("\r\n", "\n")

        if sReceived != sExpected:
            testPassed = False

        recordTestResult(
            passed=testPassed,
            testName=testName,
            testResult=testResult,
            expectedBytes=expectedFileBytes,
            rvsimInput=testFileBytes
        )

numTestRuns = numTestFailures + numTestPasses

if numTestRuns == 0:
    print(red("No tests run."))

    if arguments.specificTest:
        print("Couldn't find test '" + arguments.specificTest + "'")

    exit(1)
else:
    print(numTestPasses, "/", numTestRuns,
          " tests passed (", 100.0*numTestPasses/numTestRuns, "%).", sep='')
    exit(0)
