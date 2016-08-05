require 'cosmos/conversions/conversion'
include Math
module Cosmos
  class EreVoltage < Conversion
    def initialize(resolution, gain)
      super()
	  @max = 2**(resolution.to_f-1)-1
	  @gain = gain
    end
    def call(value, packet, buffer)
	  # convert from ADC code to volts
	  v = (value.to_f / @max.to_f) * (2.048 / @gain.to_f) * (180.0 / 33.0)
	  return v
    end
  end
end