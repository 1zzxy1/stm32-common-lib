# simple_uart.py - Simple UART management module

from maix import uart, time
from .config import config
from .utils import micu_printf, safe_decode, Timer, set_global_uart, extract_and_apply_data
import threading

class SimpleUART:
    """Simple UART manager with direct buffer refresh mode and data extraction"""
    
    def __init__(self):
        self.serial = None
        self.rx_buf = ""  # Receive buffer
        self.is_initialized = False
        self.refresh_timer = Timer(config.refresh_interval)
        self.auto_refresh = True  # Auto refresh mode
        self.auto_extract = False  # Auto extract key=value pairs
        
        # Thread safety
        self._buffer_lock = threading.Lock()  # 缓冲区锁
        self._extract_lock = threading.Lock()  # 数据提取锁
    
    def init(self, device=None, baudrate=None, set_as_global=True):
        """Initialize UART
        
        Args:
            device (str, optional): UART device path, None to use config default
            baudrate (int, optional): Baud rate, None to use config default
            set_as_global (bool): Whether to set as global UART instance for micu_printf
        """
        try:
            # Use provided parameters or config defaults
            uart_device = device if device is not None else config.uart_device
            uart_baudrate = baudrate if baudrate is not None else config.uart_baudrate
            
            self.serial = uart.UART(uart_device, uart_baudrate)
            self.serial.set_received_callback(self._on_received)
            
            self.is_initialized = True
            
            # Set as global UART instance for micu_printf
            if set_as_global:
                set_global_uart(self)
            
            print(f"UART initialized successfully - {uart_device}:{uart_baudrate}")
            if set_as_global:
                print("Set as global UART instance, micu_printf available")
            self._show_frame_config()
            
            return True
            
        except Exception as e:
            print(f"UART initialization failed: {str(e)}")
            self.is_initialized = False
            return False
    
    def set_frame(self, header="$$", tail="##", enabled=True):
        """Set global frame header and tail (used for all send/receive)
        
        Args:
            header (str): Frame header
            tail (str): Frame tail
            enabled (bool): Whether to enable frame detection
        """
        config.set_frame_format(header, tail, enabled)
        print(f"Frame format: {header}...{tail} ({'enabled' if enabled else 'disabled'})")
    
    def set_auto_refresh(self, enabled=True):
        """Set auto refresh mode
        
        Args:
            enabled (bool): Whether to enable auto refresh
        """
        self.auto_refresh = enabled
        print(f"Auto refresh: {'enabled' if enabled else 'disabled'}")
    
    def set_auto_extract(self, enabled=True):
        """Set auto data extraction mode
        
        Args:
            enabled (bool): Whether to enable auto extraction of key=value pairs
        """
        self.auto_extract = enabled
        print(f"Auto data extraction: {'enabled' if enabled else 'disabled'}")
    
    def _show_frame_config(self):
        """Show current frame format configuration"""
        frame_config = config.get_frame_config()
        if frame_config["enabled"]:
            print(f"Frame format: {frame_config['header']}...{frame_config['tail']}")
        else:
            print("Frame format: disabled (line-based processing)")
    
    def _on_received(self, serial_obj, data: bytes):
        """UART data receive callback - Thread Safe"""
        try:
            decoded_data = safe_decode(data)
            if decoded_data is None:
                return
            
            frame_config = config.get_frame_config()
            
            if frame_config["enabled"]:
                # Frame mode: extract complete frames
                clean_data = self._extract_frame_content(decoded_data, frame_config)
                if clean_data is None:
                    return  # No valid frame found
            else:
                # No frame mode: accept all data
                clean_data = decoded_data
            
            # Thread-safe buffer update
            with self._buffer_lock:
                self._update_buffer(clean_data)
            
            # Thread-safe data extraction
            if self.auto_extract and clean_data.strip():
                with self._extract_lock:
                    extract_and_apply_data(clean_data.strip())
            
        except Exception as e:
            print(f"Receive error: {str(e)}")
    
    def _extract_frame_content(self, data, frame_config):
        """Extract content from frame"""
        header = frame_config["header"]
        tail = frame_config["tail"]
        
        # Check if data contains complete frame
        if header in data and tail in data:
            # Find complete frame
            header_pos = data.find(header)
            tail_pos = data.find(tail, header_pos + len(header))
            
            if header_pos != -1 and tail_pos != -1:
                # Extract content between header and tail
                frame_content = data[header_pos + len(header):tail_pos]
                print(f"Valid frame received - content: {frame_content.strip()}")
                return frame_content.strip()
            else:
                print(f"Invalid frame format - ignoring data: {data}")
        else:
            print(f"No complete frame found - ignoring data: {data}")
        
        return None
    
    def _update_buffer(self, data):
        """Update receive buffer with new data - Called with lock held"""
        if self.auto_refresh:
            # Direct refresh mode: replace buffer with new data
            self.rx_buf = data
            print(f"Buffer refreshed - new data: {data[:50] + '...' if len(data) > 50 else data}")
        else:
            # Accumulate mode: add data to buffer
            if len(self.rx_buf) + len(data) > config.max_buffer_size:
                print("Buffer full, replacing with new data")
                self.rx_buf = data
            else:
                self.rx_buf += data
                print(f"Data added to buffer: {data}")
    
    def send(self, data):
        """Send data (using global frame header/tail settings) - Enhanced with retry
        
        Args:
            data (str): Data to send
        """
        if not self.is_initialized:
            print("UART not initialized")
            return False
        
        try:
            # Ensure input data is properly formatted
            if isinstance(data, bytes):
                data = data.decode('utf-8', errors='replace')
            else:
                data = str(data)
            
            frame_config = config.get_frame_config()
            
            if frame_config["enabled"]:
                # Use global configured frame header/tail
                frame_data = frame_config["header"] + data + frame_config["tail"]
            else:
                frame_data = data
            
            # Add line ending
            final_data = frame_data + "\r\n"
            
            # Try to send with retry mechanism
            max_retries = 3
            retry_delay = 10  # ms
            
            for attempt in range(max_retries):
                try:
                    # Send data
                    result = self.serial.write_str(final_data)
                    
                    # Check if write was successful
                    if result is not None and result >= 0:
                        return True
                    else:
                        if attempt < max_retries - 1:
                            print(f"Send attempt {attempt + 1} failed, retrying...")
                            time.sleep_ms(retry_delay)
                        else:
                            print(f"Send failed after {max_retries} attempts")
                            return False
                            
                except Exception as e:
                    if attempt < max_retries - 1:
                        print(f"Send attempt {attempt + 1} error: {e}, retrying...")
                        time.sleep_ms(retry_delay)
                    else:
                        print(f"Send failed after {max_retries} attempts: {e}")
                        return False
            
            return False
            
        except Exception as e:
            print(f"Send failed: {str(e)}")
            return False
    
    def receive(self):
        """Receive one complete data frame (using global frame header/tail settings) - Thread Safe
        
        Returns:
            str or None: Extracted data content, None if no complete frame
        """
        with self._buffer_lock:
            if not self.rx_buf:
                return None
                
            frame_config = config.get_frame_config()
            
            if not frame_config["enabled"]:
                # If frame detection disabled, return buffer data directly and clear
                data = self.rx_buf.strip()
                self.rx_buf = ""  # Clear buffer immediately
                return data if data else None
            
            # Use global configured frame header/tail
            header = frame_config["header"]
            tail = frame_config["tail"]
            
            return self._extract_frame_with_delimiters(header, tail)
    
    def receive_all(self):
        """Receive all available complete frames (using global frame header/tail settings) - Thread Safe
        
        Returns:
            list: List of all complete frame data
        """
        frames = []
        
        with self._buffer_lock:
            if not self.rx_buf:
                return frames
                
            frame_config = config.get_frame_config()
            
            if not frame_config["enabled"]:
                # If frame detection disabled, return entire buffer as one item
                if self.rx_buf.strip():
                    frames.append(self.rx_buf.strip())
                self.rx_buf = ""  # Clear buffer
                return frames
        
        # Extract all complete frames (release lock during extraction)
        while True:
            frame_data = self.receive()
            if frame_data is None:
                break
            frames.append(frame_data)
        
        return frames
    
    def extract_data_from_buffer(self):
        """Extract key:value pairs from current buffer - Thread Safe
        
        Returns:
            dict: Dictionary of applied variables
        """
        with self._buffer_lock:
            if not self.rx_buf:
                print("Buffer is empty")
                return {}
            
            buffer_copy = self.rx_buf.strip()
        
        # Extract data outside of lock
        with self._extract_lock:
            return extract_and_apply_data(buffer_copy)
    
    def _extract_frame_with_delimiters(self, header, tail):
        """Extract data with specified frame header/tail - Called with lock held"""
        # Find header
        header_pos = self.rx_buf.find(header)
        if header_pos == -1:
            return None
        
        # Find tail starting from header position
        tail_pos = self.rx_buf.find(tail, header_pos + len(header))
        if tail_pos == -1:
            return None  # No complete frame found
        
        # Extract data from frame (remove header/tail)
        data = self.rx_buf[header_pos + len(header):tail_pos]
        
        # Clear entire buffer directly (refresh mode)
        self.rx_buf = ""
        
        return data.strip()
    
    def get_buffer(self):
        """Get raw buffer content - Thread Safe"""
        with self._buffer_lock:
            return self.rx_buf
    
    def clear_buffer(self):
        """Clear receive buffer - Thread Safe"""
        with self._buffer_lock:
            self.rx_buf = ""
        print("Buffer cleared")
    
    def flush_buffer(self):
        """Force flush buffer - get and clear all data - Thread Safe"""
        with self._buffer_lock:
            data = self.rx_buf
            self.rx_buf = ""
        print("Buffer force flushed")
        return data
    
    def has_data(self):
        """Check if there is data in buffer - Thread Safe"""
        with self._buffer_lock:
            return len(self.rx_buf) > 0
    
    def buffer_size(self):
        """Get current buffer size - Thread Safe"""
        with self._buffer_lock:
            return len(self.rx_buf)
    
    def refresh(self):
        """Timer refresh processing - Thread Safe"""
        if self.refresh_timer.is_timeout():
            if self.auto_refresh:
                with self._buffer_lock:
                    if self.rx_buf:
                        # In auto refresh mode, periodically clean old data
                        print("Timer refresh: clearing buffer")
                        self.rx_buf = ""
    
    def close(self):
        """Close UART"""
        if self.serial:
            try:
                self.serial.close()
                print("UART closed")
            except:
                pass
        
        self.is_initialized = False
        self.serial = None 