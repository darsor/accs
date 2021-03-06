require 'cosmos/processors/processor'
module Cosmos

  class PowerProcessor < Processor

    # @param resolution: ADC resolution (12, 14, 16, or 18)
    def initialize(resolution)
	value_type = :CONVERTED
	  super(value_type)
	  @max = 2**(resolution.to_f-1)-1
    end

    # Run conversions on the power readings
    #
    # See Processor#call
    def call(packet, buffer)
	  @results[:VOLTAGE] = voltage(packet.read("VOLTAGE_RAW", :RAW, buffer))
	  @results[:AMPERAGE] = amperage(packet.read("AMPERAGE_RAW", :RAW, buffer))
	  @results[:POWER] = packet.read("AMPERAGE", :CONVERTED, buffer) * 28.0
    end
	
	# convert ADC code to voltage
	def voltage(code, gain=0.4936)
	  return (code.to_f / @max.to_f) * (2.048 / gain.to_f) * (180.0 / 33.0)
	end
	
	# convert ADC code to amperage
	def amperage(code, gain=8.0)
	  volts = (code.to_f / @max.to_f) * (2.048 / gain.to_f) * (180.0 / 33.0)
	  # i = V / R
	  return volts / 0.03;
	end

    # Convert to configuration file string
    def to_config
      "  PROCESSOR #{@name} #{self.class.name.to_s.class_name_to_filename} #{@item_name} #{@value_type}\n"
    end

  end # class PowerProcessor

end # module Cosmos
