# __init__.py - Module initialization file

"""
MICU UART Communication Library - Direct Buffer Refresh Mode
Provides simple and efficient UART communication functionality

Features:
- Direct buffer refresh to avoid data accumulation
- Support for global frame header/tail settings
- Provides micu_printf for UART debug output
- Data extraction and variable binding system
- Key=value pair parsing from UART data
- Simplified configuration management
- Pure ASCII/English support
"""

from .simple_uart import SimpleUART
from .utils import (
    micu_printf, set_global_uart, get_global_uart, Timer, get_timestamp,
    bind_variable, get_variable_bindings, clear_variable_bindings,
    parse_key_value_pairs, apply_parsed_data, extract_and_apply_data,
    VariableContainer, safe_decode
)
from .config import config

__version__ = "2.4.0"
__author__ = "MICU Team"

# Public interface exports
__all__ = [
    # Core classes
    'SimpleUART',
    
    # Utility functions
    'micu_printf',
    'set_global_uart', 
    'get_global_uart',
    'Timer',
    'get_timestamp',
    'safe_decode',
    
    # Data extraction and variable binding
    'bind_variable',
    'get_variable_bindings',
    'clear_variable_bindings',
    'parse_key_value_pairs',
    'apply_parsed_data',
    'extract_and_apply_data',
    'VariableContainer',
    
    # Configuration
    'config',
] 