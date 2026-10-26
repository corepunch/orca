-- Temperature unit preference.
-- Stored in module scope so the selection persists for the lifetime of the
-- app process.  Defaults to Celsius (the Open-Meteo API always returns °C).

_unit = "celsius"

get_unit = -> _unit

set_unit = (u) -> _unit = u

return { :get_unit, :set_unit }
