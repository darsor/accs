require 'cosmos/conversions/conversion'
include Math
module Cosmos
  class Level < Conversion
    def initialize(resolution, gain)
      super()
	  @max = 2**(resolution.to_f-1)-1
	  @gain = gain
    end
    def call(value, packet, buffer)
	  # convert from ADC code to volts
	  v = (value.to_f / @max.to_f) * (2.048 / @gain.to_f) * (180.0 / 33.0)
	  # convert to level in inches
	  inches = -1.6912*(v.to_f)**2.0 + 7.9319 * v.to_f + 0.2633
    end
  end
end