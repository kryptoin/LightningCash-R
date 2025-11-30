# LightningCashr Functional Tests

This directory contains functional tests for the LightningCashr blockchain project.

## Overview

The tests verify basic functionality without requiring any changes to the source code. They use isolated test environments (regtest mode) to avoid interfering with existing blockchain data.

## Prerequisites

Before running tests, ensure you have:

1. **Built the project:**
   ```bash
   ./build-osx-qt-m1.sh && gmake
   ```

2. **Python 3 installed** (Python 3.7 or later recommended)

## Running Tests

### Run All Tests

```bash
cd /Users/user/Desktop/lncr
python3 test/functional/test_runner.py
```

### Run Specific Test

```bash
python3 test/functional/test_runner.py example_test
```

### Run with Verbose Output

```bash
python3 test/functional/test_runner.py -v
```

### List Available Tests

```bash
python3 test/functional/test_runner.py --list
```

## Available Tests

1. **example_test.py** - Basic node startup and RPC functionality
   - Starts a node in regtest mode
   - Verifies blockchain and network info
   - Tests basic RPC calls

2. **wallet_basic_test.py** - Basic wallet operations
   - Tests address generation
   - Validates address format
   - Checks wallet balance and transactions

3. **cli_test.py** - Command-line tool functionality
   - Tests `lightningcashr-cli --help`
   - Verifies CLI can communicate with daemon
   - Tests error handling for invalid commands

## Test Framework Features

### Reindexing Protection

The test framework includes robust mechanisms to handle node initialization delays:

- **Isolated environments:** Each test uses a fresh regtest datadir (no reindexing needed)
- **RPC waiting:** Polls RPC with exponential backoff (up to 5 minutes)
- **Initialization checks:** Validates node is fully ready before running tests
- **Automatic cleanup:** Removes temporary directories after test completion

### Test Isolation

- Each test runs with its own temporary datadir
- Tests use unique RPC ports to avoid conflicts
- Clean slate for every test run (no shared state)

## Troubleshooting

### Test Timeout

If tests timeout, it may indicate the node is taking too long to start. Check:
- System resources (CPU/memory)
- Previous daemon processes still running: `ps aux | grep lightningcashrd`
- Disk space availability

### RPC Connection Issues

If tests fail with RPC errors:
- Verify binaries were built successfully
- Check that no other instances are using the RPC ports
- Review test logs for specific error messages

### Cleanup Issues

If temporary directories aren't cleaned up:
- Manually remove: `rm -rf /tmp/lncr_test_*`
- Check for zombie processes: `ps aux | grep lightningcashrd`

## Adding New Tests

To add a new test:

1. Create a new file in `test/functional/` (e.g., `my_test.py`)
2. Import and extend `LightningCashrTestFramework`:
   ```python
   from test_framework import LightningCashrTestFramework
   
   class MyTest(LightningCashrTestFramework):
       def run_test(self):
           # Your test logic here
           node = self.start_node(0)
           # ...
   ```
3. Add your test to `ALL_TESTS` list in `test_runner.py`
4. Make it executable: `chmod +x test/functional/my_test.py`

## Notes

- Tests do **not** modify source code
- All tests run in isolated regtest environments
- Automatic cleanup prevents test data accumulation
- Framework handles reindexing delays automatically
