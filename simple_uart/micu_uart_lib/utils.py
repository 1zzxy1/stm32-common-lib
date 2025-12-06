# utils.py - Utility functions module

from maix import time
from .config import config
import re
import threading

# Global UART instance for micu_printf
_global_uart = None

# Global variable binding registry
_variable_bindings = {}

# Thread safety for variable bindings
_bindings_lock = threading.Lock()

def set_global_uart(uart_instance):
    """Set global UART instance
    
    Args:
        uart_instance: SimpleUART instance
    """
    global _global_uart
    _global_uart = uart_instance

def get_global_uart():
    """Get global UART instance"""
    return _global_uart

def micu_printf(format_str, *args):
    """Printf-style function that sends formatted data via UART
    
    Args:
        format_str (str): Format string
        *args: Format arguments
        
    Returns:
        bool: True if sent successfully, False otherwise
    """
    try:
        # Format the string
        if args:
            output = format_str % args
        else:
            output = str(format_str)
        
        # Output to console
        print(output)
        
        # Send via UART if available
        if _global_uart and _global_uart.is_initialized:
            # Add small delay before sending to prevent buffer overflow
            time.sleep_ms(5)
            
            success = _global_uart.send(output)
            if not success:
                print("Warning: UART send failed")
            return success
        else:
            # print("Warning: UART not initialized, console output only")
            return False
            
    except Exception as e:
        # Fallback for format errors
        try:
            if args:
                safe_output = f"{format_str} {' '.join(str(arg) for arg in args)}"
            else:
                safe_output = str(format_str)
            
            print(safe_output)
            print(f"Format warning: {e}")
            
            if _global_uart and _global_uart.is_initialized:
                time.sleep_ms(5)
                return _global_uart.send(safe_output)
            return False
        except Exception as e2:
            print(f"micu_printf error: {e2}")
            return False

def bind_variable(name, var_ref, var_type='auto'):
    """Bind a variable name to a variable reference for data parsing - Thread Safe
    
    Args:
        name (str): Variable name to bind (e.g., 'micu', 'abc')
        var_ref: Variable reference or container to store parsed value
        var_type (str): Variable type - 'int', 'float', 'str', or 'auto'
    """
    global _variable_bindings
    with _bindings_lock:
        _variable_bindings[name] = {
            'ref': var_ref,
            'type': var_type
        }
    print(f"Variable binding: {name} -> {var_type}")

def get_variable_bindings():
    """Get all variable bindings - Thread Safe"""
    with _bindings_lock:
        return _variable_bindings.copy()

def clear_variable_bindings():
    """Clear all variable bindings - Thread Safe"""
    global _variable_bindings
    with _bindings_lock:
        _variable_bindings.clear()
    print("All variable bindings cleared")

def parse_key_value_pairs(data_string):
    """Parse key:value pairs from string
    
    Args:
        data_string (str): String containing key:value pairs
        
    Returns:
        dict: Dictionary of parsed key-value pairs
    """
    result = {}
    
    # Remove common separators and clean the string
    cleaned = data_string.replace(',', ' ').replace(';', ' ').replace('，', ' ')
    
    # Pattern to match key:value pairs
    pattern = r'([a-zA-Z_][a-zA-Z0-9_]*)\s*:\s*([^\s:]+)'
    matches = re.findall(pattern, cleaned)
    
    for key, value in matches:
        # Try to convert value to appropriate type
        parsed_value = _parse_value(value)
        result[key] = parsed_value
        print(f"Parsed: {key} = {parsed_value} (type: {type(parsed_value).__name__})")
    
    return result

def _parse_value(value_str):
    """Parse string value to appropriate type"""
    value_str = value_str.strip()
    
    # Try integer
    try:
        if '.' not in value_str and 'e' not in value_str.lower():
            return int(value_str)
    except ValueError:
        pass
    
    # Try float
    try:
        return float(value_str)
    except ValueError:
        pass
    
    # Return as string
    return value_str

def apply_parsed_data(parsed_data):
    """Apply parsed data to bound variables - Thread Safe
    
    Args:
        parsed_data (dict): Dictionary of parsed key-value pairs
        
    Returns:
        dict: Dictionary of successfully applied variables
    """
    applied = {}
    
    # Get current bindings in thread-safe way
    with _bindings_lock:
        current_bindings = _variable_bindings.copy()
    
    for key, value in parsed_data.items():
        if key in current_bindings:
            binding = current_bindings[key]
            var_ref = binding['ref']
            var_type = binding['type']
            
            try:
                # Convert value to specified type if needed
                if var_type == 'int':
                    converted_value = int(float(value))  # Handle "123.0" -> 123
                elif var_type == 'float':
                    converted_value = float(value)
                elif var_type == 'str':
                    converted_value = str(value)
                else:  # auto
                    converted_value = value
                
                # Apply value to variable (this should be thread-safe for VariableContainer)
                if isinstance(var_ref, dict) and 'key' in var_ref:
                    # For dictionary-style binding
                    var_ref['container'][var_ref['key']] = converted_value
                elif hasattr(var_ref, 'set'):
                    # For VariableContainer objects
                    var_ref.set(converted_value)
                elif hasattr(var_ref, '__setitem__'):
                    # For list-style binding
                    var_ref[0] = converted_value
                else:
                    # Direct assignment (note: this won't work for immutable types)
                    var_ref = converted_value
                
                applied[key] = converted_value
                print(f"Applied: {key} = {converted_value}")
                
            except Exception as e:
                print(f"Error applying {key}:{value}: {e}")
        else:
            print(f"Warning: No binding for variable '{key}'")
    
    return applied

def extract_and_apply_data(buffer_data):
    """Extract key:value pairs from buffer and apply to bound variables - Thread Safe
    
    Args:
        buffer_data (str): Buffer data containing key:value pairs
        
    Returns:
        dict: Dictionary of applied variables
    """
    print(f"Extracting data from: {buffer_data}")
    parsed = parse_key_value_pairs(buffer_data)
    applied = apply_parsed_data(parsed)
    return applied

def safe_decode(data, encoding='utf-8'):
    """Safely decode byte data"""
    if isinstance(data, str):
        return data
        
    if not isinstance(data, bytes):
        return str(data)
    
    try:
        return data.decode(encoding, errors='replace')
    except Exception:
        return str(data)

def get_timestamp():
    """Get current timestamp"""
    return time.ticks_ms()

class Timer:
    """Simple timer class"""
    
    def __init__(self, interval_ms):
        self.interval = interval_ms
        self.last_time = get_timestamp()
    
    def is_timeout(self):
        """Check if timer has timed out"""
        current_time = get_timestamp()
        if current_time - self.last_time >= self.interval:
            self.last_time = current_time
            return True
        return False
    
    def reset(self):
        """Reset timer"""
        self.last_time = get_timestamp()
    
    def set_interval(self, interval_ms):
        """Set new interval"""
        self.interval = interval_ms

class VariableContainer:
    """Helper class for variable binding - Thread Safe"""
    
    def __init__(self, initial_value=None):
        self.value = initial_value
        self._lock = threading.Lock()
    
    def set(self, new_value):
        """Set the value - Thread Safe"""
        with self._lock:
            self.value = new_value
    
    def get(self):
        """Get the value - Thread Safe"""
        with self._lock:
            return self.value
    
    def __str__(self):
        with self._lock:
            return str(self.value)
    
    def __repr__(self):
        with self._lock:
            return f"VariableContainer({self.value})" 