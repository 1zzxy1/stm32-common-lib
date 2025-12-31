# config.py - Configuration management module

class Config:
    """System configuration management class"""
    
    def __init__(self):
        # UART configuration
        self.uart_device = "/dev/ttyS0"
        self.uart_baudrate = 115200
        
        # Buffer configuration
        self.max_buffer_size = 1024
        
        # Refresh mode configuration
        self.refresh_interval = 500  # ms - Timer refresh interval
        
        # Frame format configuration
        self.frame_config = {
            "header": "$$",      # Default frame header
            "tail": "##",        # Default frame tail
            "enabled": True      # Enable frame detection
        }
    
    def set_frame_format(self, header="$$", tail="##", enabled=True):
        """Set data frame format"""
        self.frame_config["header"] = header
        self.frame_config["tail"] = tail
        self.frame_config["enabled"] = enabled
    
    def get_frame_config(self):
        """Get frame configuration"""
        return self.frame_config.copy()
    
    def update_config(self, **kwargs):
        """Batch update configuration"""
        for key, value in kwargs.items():
            if hasattr(self, key):
                setattr(self, key, value)

# Global configuration instance
config = Config() 