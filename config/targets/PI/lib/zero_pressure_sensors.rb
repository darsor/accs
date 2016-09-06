require 'cosmos/conversions/conversion'
module Cosmos
  class ZeroPressureSensors < Conversion
    def initialize()
      super()
    end
    def call(value, packet, buffer)
	  $start_zero = 1
	  sleep(1)
	  return 1
    end
  end
end