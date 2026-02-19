"""
ORCA Plugin System

This module provides the base classes and registry for the plugin architecture.
"""

class BasePlugin:
	"""Base class for all plugins"""
	def close(self):
		"""Optional close method for cleanup"""
		pass

class OutputPlugin(BasePlugin):
	"""Base class for file-based output plugins"""
	def __init__(self, filename, extension):
		self.filename = filename
		self.extension = extension
		self.file_handle = None
	
	def open(self):
		"""Open the output file"""
		output_filename = self.filename.replace(".xml", self.extension)
		self.file_handle = open(output_filename, "w")
		return self.file_handle
	
	def close(self):
		"""Close the output file"""
		if self.file_handle:
			self.file_handle.close()
	
	def write(self, text):
		"""Write a line to the output file"""
		if self.file_handle:
			self.file_handle.write(text)
			self.file_handle.write("\n")
	
	def get_handle(self):
		"""Get the file handle for direct writing"""
		return self.file_handle

class PluginRegistry:
	"""Registry for managing output plugins"""
	_plugins = {}
	_enabled_plugins = set()
	
	@classmethod
	def register(cls, name, plugin_class):
		"""Register a new plugin type"""
		cls._plugins[name] = plugin_class
		cls._enabled_plugins.add(name)
	
	@classmethod
	def unregister(cls, name):
		"""Unregister a plugin type"""
		if name in cls._plugins:
			del cls._plugins[name]
			cls._enabled_plugins.discard(name)
	
	@classmethod
	def enable_plugin(cls, name):
		"""Enable a registered plugin"""
		if name in cls._plugins:
			cls._enabled_plugins.add(name)
	
	@classmethod
	def disable_plugin(cls, name):
		"""Disable a registered plugin"""
		cls._enabled_plugins.discard(name)
	
	@classmethod
	def get_enabled_plugins(cls):
		"""Get list of enabled plugin names"""
		return cls._enabled_plugins.copy()
	
	@classmethod
	def create_plugin(cls, name, *args, **kwargs):
		"""Create an instance of a plugin"""
		if name in cls._plugins and name in cls._enabled_plugins:
			return cls._plugins[name](*args, **kwargs)
		return None
	
	@classmethod
	def get_plugin_class(cls, name):
		"""Get the plugin class by name"""
		return cls._plugins.get(name)

# Export public API
__all__ = ['BasePlugin', 'OutputPlugin', 'PluginRegistry']
