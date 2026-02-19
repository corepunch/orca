"""
Header Plugin - Generates C header files (.h)
"""

from . import OutputPlugin

class HeaderPlugin(OutputPlugin):
	"""Plugin for generating C header files (.h)"""
	def __init__(self, filename):
		super().__init__(filename, ".h")
