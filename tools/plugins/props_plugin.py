"""
Props Plugin - Generates C property files (_properties.h)
"""

from . import OutputPlugin

class PropsPlugin(OutputPlugin):
	"""Plugin for generating C property files (_properties.h)"""
	def __init__(self, filename):
		super().__init__(filename, "_properties.h")
