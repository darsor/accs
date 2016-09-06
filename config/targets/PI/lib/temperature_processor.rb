require 'cosmos/processors/processor'
include Math
module Cosmos

  class TemperatureProcessor < Processor

    # @param resolution: ADC resolution (12, 14, 16, or 18)
    def initialize()
	value_type = :CONVERTED
	  super(value_type)
	  @a = 0.0039083
	  @b = -0.0000005775
    end

    # Run conversions on the pressure readings
    #
    # See Processor#call
    def call(packet, buffer)
	  @results[:TEMP1_OHMS] = resistance(packet.read("TEMP1_RAW", :RAW, buffer))
	  @results[:TEMP1] = temperature(@results[:TEMP1_OHMS])
	  
	  @results[:TEMP2_OHMS] = resistance(packet.read("TEMP2_RAW", :RAW, buffer))
	  @results[:TEMP2] = temperature(@results[:TEMP2_OHMS])
	  
	  @results[:TEMP3_OHMS] = resistance(packet.read("TEMP3_RAW", :RAW, buffer))
	  @results[:TEMP3] = temperature(@results[:TEMP3_OHMS])
	  
	  @results[:TEMP4_OHMS] = resistance(packet.read("TEMP4_RAW", :RAW, buffer))
	  @results[:TEMP4] = temperature(@results[:TEMP4_OHMS])
    end
	
	# convert ohms to temperature
	def temperature(ohms)
	  return (sqrt((@a.to_f**2.0) - 4.0 * @b.to_f * (1 - (ohms.to_f/100.0))) - @a.to_f) / (2 * @b.to_f)
	end
	
	# convert ADC code to ohms
	def resistance(code)
	  # this 2.162 should be changed to match whatever the reference resistance is in kilo-ohms.
	  return (code.to_f / 32767.0) * (2.162 / 32.0) / 0.0005
	end

    # Convert to configuration file string
    def to_config
      "  PROCESSOR #{@name} #{self.class.name.to_s.class_name_to_filename} #{@item_name} #{@value_type}\n"
    end

  end # class PressureProcessor

end # module Cosmos
