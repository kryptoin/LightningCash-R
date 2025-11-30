#!/usr/bin/env python3
# Copyright (c) 2025 LightningCashr Test Framework
# Distributed under the MIT software license

"""
Test runner for LightningCashr functional tests.

Run all tests:
    python3 test_runner.py

Run specific test:
    python3 test_runner.py example_test
"""

import sys
import os
import argparse
import subprocess
import time
from pathlib import Path

# ANSI color codes for output
GREEN = '\033[92m'
RED = '\033[91m'
YELLOW = '\033[93m'
BLUE = '\033[94m'
RESET = '\033[0m'

# List of all functional tests
ALL_TESTS = [
    'example_test.py',
    'wallet_basic_test.py',
    'cli_test.py',
]

def run_test(test_file, verbose=False):
    """Run a single test file."""
    test_path = Path(__file__).parent / test_file
    
    if not test_path.exists():
        print(f"{RED}✗{RESET} Test file not found: {test_file}")
        return False
    
    print(f"{BLUE}Running:{RESET} {test_file}...", flush=True)
    start_time = time.time()
    
    try:
        # Run the test
        result = subprocess.run(
            [sys.executable, str(test_path)],
            capture_output=not verbose,
            text=True,
            timeout=600  # 10 minute timeout per test
        )
        
        elapsed = time.time() - start_time
        
        if result.returncode == 0:
            print(f"{GREEN}✓ PASSED{RESET} {test_file} ({elapsed:.1f}s)")
            return True
        else:
            print(f"{RED}✗ FAILED{RESET} {test_file} ({elapsed:.1f}s)")
            if not verbose and result.stdout:
                print(f"  stdout: {result.stdout[:200]}")
            if not verbose and result.stderr:
                print(f"  stderr: {result.stderr[:200]}")
            return False
            
    except subprocess.TimeoutExpired:
        print(f"{RED}✗ TIMEOUT{RESET} {test_file}")
        return False
    except Exception as e:
        print(f"{RED}✗ ERROR{RESET} {test_file}: {e}")
        return False

def main():
    """Main test runner."""
    parser = argparse.ArgumentParser(
        description='Run LightningCashr functional tests',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  Run all tests:
    python3 test_runner.py
    
  Run specific test:
    python3 test_runner.py example_test
    
  Run with verbose output:
    python3 test_runner.py -v
        """
    )
    parser.add_argument(
        'tests',
        nargs='*',
        help='Specific tests to run (without .py extension)'
    )
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Show verbose test output'
    )
    parser.add_argument(
        '--list',
        action='store_true',
        help='List all available tests'
    )
    
    args = parser.parse_args()
    
    # List tests if requested
    if args.list:
        print("Available tests:")
        for test in ALL_TESTS:
            print(f"  - {test[:-3]}")  # Remove .py extension
        return 0
    
    # Determine which tests to run
    if args.tests:
        # Run specific tests
        tests_to_run = []
        for test_name in args.tests:
            # Add .py if not present
            if not test_name.endswith('.py'):
                test_name = test_name + '.py'
            
            if test_name in ALL_TESTS:
                tests_to_run.append(test_name)
            else:
                print(f"{YELLOW}Warning:{RESET} Test not found: {test_name}")
        
        if not tests_to_run:
            print(f"{RED}Error:{RESET} No valid tests specified")
            return 1
    else:
        # Run all tests
        tests_to_run = ALL_TESTS
    
    # Run the tests
    print(f"\n{BLUE}LightningCashr Functional Test Runner{RESET}")
    print(f"Running {len(tests_to_run)} test(s)...\n")
    
    passed = 0
    failed = 0
    
    start_time = time.time()
    
    for test_file in tests_to_run:
        if run_test(test_file, args.verbose):
            passed += 1
        else:
            failed += 1
        print()  # Blank line between tests
    
    total_time = time.time() - start_time
    
    # Print summary
    print("=" * 60)
    print(f"Test Results:")
    print(f"  {GREEN}Passed:{RESET} {passed}")
    if failed > 0:
        print(f"  {RED}Failed:{RESET} {failed}")
    print(f"  Total time: {total_time:.1f}s")
    print("=" * 60)
    
    # Return exit code
    if failed > 0:
        return 1
    else:
        print(f"\n{GREEN}All tests passed!{RESET}")
        return 0

if __name__ == '__main__':
    sys.exit(main())
