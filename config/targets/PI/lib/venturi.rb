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
	  @a1 = PI*(@d1*@in2m)**2/4
	  @a2 = PI*(@d2*@in2m)**2/4
    end
    def call(value, packet, buffer)
	  # convert from ADC code to mA
	  mA = (value / @max) * (2.048 / 2.0) * (180 / 33) / 0.249
	  # convert from mA to psi
	  diff = (mA-4)/8 # tune this to actual full-scale
	  # convert from psi to mass flow rate (g/s)
	  mdot = @rho * @cd * sqrt((2*(diff * @psi2a)/@rho))*@a1/(sqrt((@a1/@a2)**2-1)) * 1000
      return mdot
    end
  end
end