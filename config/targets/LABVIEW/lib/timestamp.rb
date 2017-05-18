require 'cosmos/conversions/conversion'
module Cosmos
  class Timestamp < Conversion
    def call(value, packet, buffer)
      return value/1000000.0
    end
  end
end