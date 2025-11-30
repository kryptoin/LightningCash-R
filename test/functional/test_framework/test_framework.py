#!/usr/bin/env python3
# Copyright (c) 2025 LightningCashr Test Framework
# Distributed under the MIT software license

"""Base class for functional tests."""

import os
import sys
import logging
import tempfile

# Import utilities from same directory
import util
from util import (
    create_temp_datadir,
    start_node,
    stop_node,
    wait_for_rpc_ready,
    is_node_ready,
    rpc_call,
    cleanup_datadir
)

# Setup logging
logging.basicConfig(
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
    level=logging.INFO
)

class LightningCashrTestFramework:
    """Base class for LightningCashr functional tests."""
    
    def __init__(self):
        self.nodes = []
        self.tempdirs = []
        self.logger = logging.getLogger(self.__class__.__name__)
        self.binary_dir = None
        self.setup()
    
    def setup(self):
        """Override this to set up test-specific configuration."""
        pass
    
    def get_binary_path(self, binary_name="lightningcashrd"):
        """Get the path to a binary executable."""
        if self.binary_dir:
            binary_path = os.path.join(self.binary_dir, binary_name)
        else:
            # Try to find binary in common locations
            script_dir = os.path.dirname(os.path.abspath(__file__))
            project_root = os.path.abspath(os.path.join(script_dir, "..", "..", ".."))
            binary_path = os.path.join(project_root, "src", binary_name)
        
        if not os.path.exists(binary_path):
            raise Exception(f"Binary not found: {binary_path}. Did you run 'make'?")
        
        return binary_path
    
    def start_node(self, node_index=0, extra_args=None):
        """
        Start a node and wait for it to be ready.
        Returns a dict with node info.
        """
        if extra_args is None:
            extra_args = []
        
        # Create temporary datadir
        datadir = create_temp_datadir(prefix=f"lncr_test_{self.__class__.__name__}_")
        self.tempdirs.append(datadir)
        
        # Use unique port for each node
        rpc_port = 18443 + node_index
        
        # Get binary path
        binary_path = self.get_binary_path("lightningcashrd")
        
        # Start the node
        self.logger.info(f"Starting node {node_index}...")
        start_node(binary_path, datadir, extra_args, rpc_port)
        
        # Wait for RPC to be ready
        wait_for_rpc_ready("127.0.0.1", rpc_port, datadir)
        
        # Verify node is fully initialized
        if not is_node_ready("127.0.0.1", rpc_port, datadir):
            raise Exception("Node started but is not ready")
        
        node_info = {
            'index': node_index,
            'host': '127.0.0.1',
            'port': rpc_port,
            'datadir': datadir,
        }
        
        self.nodes.append(node_info)
        self.logger.info(f"Node {node_index} started and ready on port {rpc_port}")
        
        return node_info
    
    def stop_node(self, node_index):
        """Stop a specific node."""
        if node_index >= len(self.nodes):
            raise Exception(f"Node {node_index} does not exist")
        
        node = self.nodes[node_index]
        self.logger.info(f"Stopping node {node_index}...")
        stop_node(node['host'], node['port'], node['datadir'])
    
    def rpc(self, node_index, method, params=None):
        """Make an RPC call to a node."""
        if params is None:
            params = []
        
        if node_index >= len(self.nodes):
            raise Exception(f"Node {node_index} does not exist")
        
        node = self.nodes[node_index]
        return rpc_call(node['host'], node['port'], method, params, node['datadir'])
    
    def run_test(self):
        """Override this with your test logic."""
        raise NotImplementedError("Test must implement run_test()")
    
    def cleanup(self):
        """Clean up all nodes and temporary directories."""
        self.logger.info("Cleaning up test environment...")
        
        # Stop all nodes
        for i in range(len(self.nodes)):
            try:
                self.stop_node(i)
            except Exception as e:
                self.logger.warning(f"Error stopping node {i}: {e}")
        
        # Clean up temp directories
        for tempdir in self.tempdirs:
            try:
                cleanup_datadir(tempdir)
            except Exception as e:
                self.logger.warning(f"Error cleaning up {tempdir}: {e}")
    
    def main(self):
        """Main test execution."""
        success = False
        try:
            self.logger.info(f"Running test: {self.__class__.__name__}")
            self.run_test()
            self.logger.info(f"✓ Test passed: {self.__class__.__name__}")
            success = True
        except AssertionError as e:
            self.logger.error(f"✗ Test failed: {e}")
            raise
        except Exception as e:
            self.logger.error(f"✗ Test error: {e}")
            raise
        finally:
            self.cleanup()
        
        return success
