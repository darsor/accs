require 'cosmos/conversions/conversion'
include Math
module Cosmos
  class Venturi < Conversion
    def initialize(resolution, rho, cd)
      super()
	  @max = 2**(resolution.to_f-1)-1
      @rho = rho.to_f
	  @cd = cd.to_f
	  @psi2pa = 6894.75729
	  @in2m = 0.0254
	  @d1 = 0.269	# inches
	  @d2 = 0.14 	# inches
	  @a1 = PI*(@d1*@in2m)**2/4.0
	  @a2 = PI*(@d2*@in2m)**2/4.0
    end
    def call(value, packet, buffer)
	  # convert from ADC code to mA
	  mA = (value.to_f / @max.to_f) * (2.048 / 2.0) * (180.0 / 33.0) / 0.249
	  if mA < 4
	    return 0.0
	  end
	  # convert from mA to psi
	  diff = (mA-4)/8.0 # tune this to actual full-scale
	  # convert from psi to volume flow rate
	  q = @cd.to_f * sqrt((2.0*(diff.to_f * @psi2pa.to_f)/@rho.to_f))*@a1.to_f/(sqrt((@a1.to_f/@a2.to_f)**2.0-1.0)) * 60.0 * 1000000.0
      return q
    end
  end
end