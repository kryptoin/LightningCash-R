#!/usr/bin/env python3
# Copyright (c) 2025 LightningCashr Test Framework
# Distributed under the MIT software license

"""Utility functions for test framework."""

import os
import sys
import time
import tempfile
import subprocess
import http.client
import json
import logging
from decimal import Decimal

logger = logging.getLogger("TestFramework.util")

# Maximum time to wait for node initialization (5 minutes)
MAX_WAIT_TIME = 300

def get_auth_cookie(datadir):
    """Read RPC authentication cookie from datadir."""
    cookie_file = os.path.join(datadir, "regtest", ".cookie")
    if not os.path.exists(cookie_file):
        return None
    with open(cookie_file, 'r') as f:
        return f.read().strip()

def rpc_call(host, port, method, params=[], datadir=None):
    """Make a JSON-RPC call to the node."""
    auth = get_auth_cookie(datadir)
    if not auth:
        raise Exception("Authentication cookie not found")
    
    headers = {
        'Content-Type': 'application/json',
    }
    
    payload = json.dumps({
        "jsonrpc": "1.0",
        "id": "test",
        "method": method,
        "params": params
    })
    
    try:
        conn = http.client.HTTPConnection(host, port, timeout=10)
        auth_header = f"Basic {auth}" if ':' not in auth else f"Basic {auth}"
        # For cookie auth, the format is __cookie__:password
        if auth and ':' in auth:
            import base64
            auth_header = "Basic " + base64.b64encode(auth.encode()).decode()
        headers['Authorization'] = auth_header
        
        conn.request("POST", "/", payload, headers)
        response = conn.getresponse()
        data = response.read().decode()
        conn.close()
        
        result = json.loads(data)
        if result.get('error'):
            raise Exception(f"RPC Error: {result['error']}")
        return result.get('result')
    except Exception as e:
        raise Exception(f"RPC call failed: {e}")

def wait_for_rpc_ready(host, port, datadir, timeout=MAX_WAIT_TIME):
    """
    Wait for RPC to be ready with exponential backoff.
    Handles cases where node is reindexing or warming up.
    """
    logger.info(f"Waiting for RPC to be ready on {host}:{port}...")
    start_time = time.time()
    retry_delay = 0.5
    max_retry_delay = 5.0
    
    while time.time() - start_time < timeout:
        try:
            # Try to get blockchain info
            result = rpc_call(host, port, "getblockchaininfo", [], datadir)
            
            # Check if node is still initializing/reindexing
            if isinstance(result, dict):
                # Check for warmup/initialization status
                if 'initialblockdownload' in result:
                    if result.get('initialblockdownload', False):
                        logger.debug(f"Node still in initial block download, waiting... ({int(time.time() - start_time)}s)")
                        time.sleep(retry_delay)
                        retry_delay = min(retry_delay * 1.5, max_retry_delay)
                        continue
                
                # If we got a valid response, we're ready
                logger.info(f"RPC ready after {int(time.time() - start_time)}s")
                return True
                
        except Exception as e:
            error_msg = str(e).lower()
            
            # Check for specific initialization errors
            if 'verifying blocks' in error_msg or 'reindexing' in error_msg:
                logger.debug(f"Node is reindexing, waiting... ({int(time.time() - start_time)}s)")
            elif 'loading' in error_msg or 'warmup' in error_msg:
                logger.debug(f"Node is warming up, waiting... ({int(time.time() - start_time)}s)")
            elif 'connection refused' in error_msg or 'authentication' in error_msg:
                logger.debug(f"RPC not yet available, waiting... ({int(time.time() - start_time)}s)")
            else:
                logger.debug(f"RPC error: {e}")
            
            time.sleep(retry_delay)
            retry_delay = min(retry_delay * 1.5, max_retry_delay)
            continue
    
    raise Exception(f"RPC did not become ready within {timeout} seconds")

def is_node_ready(host, port, datadir):
    """Check if node is fully ready (not in warmup/reindex state)."""
    try:
        result = rpc_call(host, port, "getblockchaininfo", [], datadir)
        
        # Check various readiness indicators
        if isinstance(result, dict):
            # For regtest mode, we expect the chain to be ready immediately
            if result.get('chain') == 'regtest':
                return True
            
            # Check if not in initial block download
            if not result.get('initialblockdownload', True):
                return True
                
        return False
    except Exception as e:
        logger.debug(f"Node readiness check failed: {e}")
        return False

def create_temp_datadir(prefix="test_"):
    """Create a temporary directory for test data."""
    temp_dir = tempfile.mkdtemp(prefix=prefix)
    logger.debug(f"Created temporary datadir: {temp_dir}")
    return temp_dir

def start_node(binary_path, datadir, extra_args=None, rpc_port=18443):
    """
    Start a lightningcashrd node.
    Returns the process object.
    """
    if extra_args is None:
        extra_args = []
    
    # Build command line
    cmd = [
        binary_path,
        f"-datadir={datadir}",
        "-regtest",
        f"-rpcport={rpc_port}",
        "-server",
        "-daemon",
        "-debug=rpc",
    ] + extra_args
    
    logger.info(f"Starting node: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        if result.returncode != 0:
            raise Exception(f"Failed to start node: {result.stderr}")
        logger.debug(f"Node start output: {result.stdout}")
        return True
    except subprocess.TimeoutExpired:
        raise Exception("Node startup timed out")
    except Exception as e:
        raise Exception(f"Failed to start node: {e}")

def stop_node(host, port, datadir, timeout=30):
    """Stop a running node gracefully."""
    logger.info(f"Stopping node on {host}:{port}...")
    try:
        rpc_call(host, port, "stop", [], datadir)
        
        # Wait for node to stop
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                rpc_call(host, port, "getblockchaininfo", [], datadir)
                time.sleep(0.5)
            except:
                logger.info("Node stopped successfully")
                return True
        
        raise Exception("Node did not stop within timeout")
    except Exception as e:
        # If RPC call fails, node might already be stopped
        logger.debug(f"Stop node error (might be already stopped): {e}")
        return True

def cleanup_datadir(datadir):
    """Clean up temporary data directory."""
    import shutil
    if os.path.exists(datadir):
        logger.debug(f"Cleaning up datadir: {datadir}")
        shutil.rmtree(datadir)
