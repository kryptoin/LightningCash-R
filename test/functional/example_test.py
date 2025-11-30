#!/usr/bin/env python3
# Copyright (c) 2025 LightningCashr Test Framework
# Distributed under the MIT software license

"""Example test demonstrating basic node startup and RPC functionality."""

import sys
import os

# Add test framework to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'test_framework'))

from test_framework import LightningCashrTestFramework

class ExampleTest(LightningCashrTestFramework):
    """Simple example test that starts a node and checks basic info."""
    
    def setup(self):
        """Test-specific setup."""
        self.num_nodes = 1
    
    def run_test(self):
        """Run the actual test."""
        self.logger.info("Starting example test...")
        
        # Start a single node
        node = self.start_node(0)
        self.logger.info(f"Node started on port {node['port']}")
        
        # Get blockchain info
        blockchain_info = self.rpc(0, "getblockchaininfo")
        self.logger.info(f"Blockchain info: {blockchain_info}")
        
        # Verify we're on regtest chain
        assert blockchain_info['chain'] == 'regtest', \
            f"Expected regtest chain, got {blockchain_info['chain']}"
        
        # Verify we start at block 0
        assert blockchain_info['blocks'] == 0, \
            f"Expected 0 blocks, got {blockchain_info['blocks']}"
        
        self.logger.info("✓ Blockchain info looks correct")
        
        # Get network info
        network_info = self.rpc(0, "getnetworkinfo")
        self.logger.info(f"Network info: version={network_info.get('version')}, "
                        f"subversion={network_info.get('subversion')}")
        
        assert 'version' in network_info, "Network info missing version"
        
        self.logger.info("✓ Network info looks correct")
        
        # Get wallet info (if wallet is enabled)
        try:
            wallet_info = self.rpc(0, "getwalletinfo")
            self.logger.info(f"Wallet balance: {wallet_info.get('balance', 0)}")
            assert wallet_info['balance'] == 0, "Wallet should start with 0 balance"
            self.logger.info("✓ Wallet info looks correct")
        except Exception as e:
            self.logger.info(f"Wallet not available (expected if wallet disabled): {e}")
        
        self.logger.info("Example test completed successfully!")

if __name__ == '__main__':
    test = ExampleTest()
    try:
        test.main()
        sys.exit(0)
    except Exception:
        sys.exit(1)
