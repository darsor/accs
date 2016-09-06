require 'cosmos/processors/processor'
include Math
module Cosmos

  class LevelProcessor < Processor

    # @param resolution: ADC resolution (12, 14, 16, or 18)
    def initialize(resolution)
	value_type = :CONVERTED
	  super(value_type)
	  @max = 2**(resolution.to_f-1)-1
    end

    # Run conversions on the pressure readings
    #
    # See Processor#call
    def call(packet, buffer)
	  @results[:LEVEL_VOLTS] = voltage(packet.read("LEVEL_RAW", :RAW, buffer))
	  @results[:LEVEL_INCHES] = level(@results[:LEVEL_VOLTS])
    end
	
	# convert ADC code to voltage
	def voltage(code, gain=8.0)
	  return (code.to_f / @max.to_f) * (2.048 / gain.to_f) * (180.0 / 33.0)
	end
	
	# convert volts to inches (level sensor)
	def level(volts)
	  return -1.6912*(volts.to_f)**2.0 + 7.9319 * volts.to_f + 0.2633
	end

    # Convert to configuration file string
    def to_config
      "  PROCESSOR #{@name} #{self.class.name.to_s.class_name_to_filename} #{@item_name} #{@value_type}\n"
    end

  end # class PressureProcessor

end # module Cosmos
