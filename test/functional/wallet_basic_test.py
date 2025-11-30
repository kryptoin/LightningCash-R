#!/usr/bin/env python3
# Copyright (c) 2025 LightningCashr Test Framework
# Distributed under the MIT software license

"""Test basic wallet functionality."""

import sys
import os

# Add test framework to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'test_framework'))

from test_framework import LightningCashrTestFramework

class WalletBasicTest(LightningCashrTestFramework):
    """Test basic wallet operations."""
    
    def setup(self):
        """Test-specific setup."""
        self.num_nodes = 1
    
    def run_test(self):
        """Run the actual test."""
        self.logger.info("Starting wallet basic test...")
        
        # Start node
        node = self.start_node(0)
        self.logger.info(f"Node started on port {node['port']}")
        
        # Check wallet is loaded
        try:
            wallet_info = self.rpc(0, "getwalletinfo")
            self.logger.info(f"Wallet info: {wallet_info}")
        except Exception as e:
            self.logger.warning(f"Wallet might not be enabled: {e}")
            self.logger.info("Skipping wallet tests (wallet not available)")
            return
        
        # Verify initial balance is 0
        assert wallet_info['balance'] == 0, \
            f"Expected 0 initial balance, got {wallet_info['balance']}"
        self.logger.info("✓ Initial balance is 0")
        
        # Get a new address
        address = self.rpc(0, "getnewaddress")
        self.logger.info(f"Generated new address: {address}")
        
        # Verify address is not empty
        assert address and len(address) > 0, "Address should not be empty"
        self.logger.info("✓ Successfully generated new address")
        
        # Validate the address
        validate_result = self.rpc(0, "validateaddress", [address])
        self.logger.info(f"Address validation: {validate_result}")
        
        assert validate_result['isvalid'], \
            f"Address {address} should be valid"
        assert validate_result.get('ismine', False), \
            f"Address {address} should belong to our wallet"
        
        self.logger.info("✓ Address is valid and belongs to wallet")
        
        # Get another address to verify we can generate multiple
        address2 = self.rpc(0, "getnewaddress")
        assert address2 != address, "Should generate different addresses"
        self.logger.info("✓ Can generate multiple unique addresses")
        
        # List transactions (should be empty)
        transactions = self.rpc(0, "listtransactions")
        assert len(transactions) == 0, \
            f"Expected 0 transactions, got {len(transactions)}"
        self.logger.info("✓ Transaction list is empty as expected")
        
        self.logger.info("Wallet basic test completed successfully!")

if __name__ == '__main__':
    test = WalletBasicTest()
    try:
        test.main()
        sys.exit(0)
    except Exception:
        sys.exit(1)
