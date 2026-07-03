#!/usr/bin/env python3
# Copyright (c) 2025 LightningCashr Test Framework
# Distributed under the MIT software license

"""Test CLI tool functionality."""

import sys
import os
import subprocess

# Add test framework to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'test_framework'))

from test_framework import LightningCashrTestFramework

class CLITest(LightningCashrTestFramework):
    """Test the lightningcashr-cli command-line tool."""

    def setup(self):
        """Test-specific setup."""
        self.num_nodes = 1

    def run_test(self):
        """Run the actual test."""
        self.logger.info("Starting CLI test...")

        # Get CLI binary path
        cli_binary = self.get_binary_path("lightningcashr-cli")
        self.logger.info(f"CLI binary: {cli_binary}")

        # Test 1: CLI help (doesn't require daemon)
        self.logger.info("Testing CLI help...")
        result = subprocess.run(
            [cli_binary, "--help"],
            capture_output=True,
            text=True,
            timeout=10
        )

        assert result.returncode == 0, \
            f"CLI --help should succeed, got return code {result.returncode}"
        assert len(result.stdout) > 0, "CLI help should produce output"
        assert "lightningcashr-cli" in result.stdout.lower() or "bitcoin-cli" in result.stdout.lower(), \
            "Help output should mention CLI tool"

        self.logger.info("✓ CLI help works")

        # Start a node for remaining tests
        node = self.start_node(0)
        self.logger.info(f"Node started on port {node['port']}")

        # Test 2: Basic CLI command with running daemon
        self.logger.info("Testing CLI getblockchaininfo...")
        result = subprocess.run(
            [cli_binary, "-regtest", f"-rpcport={node['port']}",
             f"-datadir={node['datadir']}", "getblockchaininfo"],
            capture_output=True,
            text=True,
            timeout=30
        )

        assert result.returncode == 0, \
            f"CLI getblockchaininfo should succeed, got: {result.stderr}"
        assert len(result.stdout) > 0, "CLI should produce output"
        assert "chain" in result.stdout, "Output should contain chain info"

        self.logger.info("✓ CLI can communicate with daemon")

        # Test 3: Invalid command should fail gracefully
        self.logger.info("Testing CLI with invalid command...")
        result = subprocess.run(
            [cli_binary, "-regtest", f"-rpcport={node['port']}",
             f"-datadir={node['datadir']}", "invalidcommandxyz"],
            capture_output=True,
            text=True,
            timeout=30
        )

        assert result.returncode != 0, \
            "CLI with invalid command should fail"
        assert len(result.stderr) > 0 or len(result.stdout) > 0, \
            "CLI should produce error message"

        self.logger.info("✓ CLI handles invalid commands correctly")

        # Test 4: Test another common command
        self.logger.info("Testing CLI getnetworkinfo...")
        result = subprocess.run(
            [cli_binary, "-regtest", f"-rpcport={node['port']}",
             f"-datadir={node['datadir']}", "getnetworkinfo"],
            capture_output=True,
            text=True,
            timeout=30
        )

        assert result.returncode == 0, \
            f"CLI getnetworkinfo should succeed"
        assert "version" in result.stdout, \
            "Network info should contain version"

        self.logger.info("✓ CLI getnetworkinfo works")

        self.logger.info("CLI test completed successfully!")

if __name__ == '__main__':
    test = CLITest()
    try:
        test.main()
        sys.exit(0)
    except Exception:
        sys.exit(1)
