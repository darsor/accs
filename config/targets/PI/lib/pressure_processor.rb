require 'cosmos/processors/processor'
include Math
module Cosmos

  class PressureProcessor < Processor

    # @param resolution: ADC resolution (12, 14, 16, or 18)
    # @param rho: density of fluid for flow rate calculation
	# @param cd: conversion coefficient for flow rate calculation
    def initialize(resolution, rho, cd)
	value_type = :CONVERTED
	  super(value_type)
	  @cd = cd.to_f
	  @max = 2**(resolution.to_f-1)-1
	  @rho = rho.to_f
	  @psi2pa = 6894.75729
	  @in2m = 0.0254
	  d1 = 0.269	# inches
	  d2 = 0.14 	# inches
	  @a1 = PI*(d1*@in2m)**2/4.0
	  @a2 = PI*(d2*@in2m)**2/4.0
	  @@static_zero = 0
	  @@venturi_zero = 0
	  @@pump_zero = 0
	  @static_zero_values = Array.new(50)
	  @venturi_zero_values = Array.new(50)
	  @pump_zero_values = Array.new(50)
	  @zero_index = 0
    end

    # Run conversions on the pressure readings
    #
    # See Processor#call
    def call(packet, buffer)
	  @results[:VENTURI_VOLTS] = voltage(packet.read("VENTURI_RAW", :RAW, buffer))
	  @results[:VENTURI_CURRENT] = milliAmps(@results[:VENTURI_VOLTS])
	  @results[:VENTURI_PRESSURE] = venturiPSI(@results[:VENTURI_CURRENT], packet.read("VENTURI_ZERO", :RAW, buffer).to_f)
	  @results[:VENTURI_FLOW] = flow(@results[:VENTURI_PRESSURE], @rho, @cd)
	  
	  @results[:STATIC_VOLTS] = voltage(packet.read("STATIC_RAW", :RAW, buffer))
	  @results[:STATIC_CURRENT] = milliAmps(@results[:STATIC_VOLTS])
	  @results[:STATIC_PRESSURE] = staticPSI(@results[:STATIC_CURRENT], packet.read("STATIC_ZERO", :RAW, buffer).to_f)
	  
	  @results[:PUMP_VOLTS] = voltage(packet.read("PUMP_RAW", :RAW, buffer))
	  @results[:PUMP_CURRENT] = milliAmps(@results[:PUMP_VOLTS])
	  @results[:PUMP_PRESSURE] = pumpPSI(@results[:PUMP_CURRENT], packet.read("PUMP_ZERO", :RAW, buffer).to_f)
	  
	  if (defined?($start_zero) != nil)
	    if ($start_zero == 1)
		  if @zero_index == 0
		    puts "Starting zero operation for pressure sensors. Don't touch anything for about 10 seconds while I average 50 values"
		  end
		  @venturi_zero_values[@zero_index] = @results[:VENTURI_PRESSURE].to_f
		  @pump_zero_values[@zero_index] = @results[:PUMP_PRESSURE].to_f
		  @static_zero_values[@zero_index] = @results[:STATIC_PRESSURE].to_f
		  @zero_index += 1
		end
	  end
	  if @zero_index == 50
	    puts "Finished zero operation for pressure sensors:"
	    @zero_index = 0
		$start_zero = 0
		@@static_zero = (@static_zero_values.inject(0, &:+)).to_f / (@static_zero_values.size.to_f)
		@@venturi_zero = (@venturi_zero_values.inject(0, &:+)).to_f / (@venturi_zero_values.size.to_f)
		@@pump_zero = (@pump_zero_values.inject(0, &:+)).to_f / (@pump_zero_values.size.to_f)
		puts "Static pressure zeroed at #{@@static_zero}"
		puts "Venturi pressure zeroed at #{@@venturi_zero}"
		puts "Pump pressure zeroed at #{@@pump_zero}"
	  end
    end
	
	# convert ADC code to voltage
	def voltage(code, gain=2.0)
	  return (code.to_f / @max.to_f) * (2.048 / gain.to_f) * (180.0 / 33.0)
	end
	
	# convert ADC voltage to milli-amps
	def milliAmps(volts)
	  return volts.to_f / 0.249
	end
	
	# convert mA to PSI (static pressure)
	def staticPSI(mAmps, zero)
	  return 1.8748 * mAmps.to_f - 7.4685 - zero.to_f
	end
	
	# convert mA to PSI (venturi pressure)
	def venturiPSI(mAmps, zero)
	  #puts "The value of @venturi_zero is #{@venturi_zero}"
	  return 0.1226 * mAmps.to_f - 0.3878 - zero.to_f
	end
	
	# convert mA to PSI (pump pressure)
	def pumpPSI(mAmps, zero)
	  return 0.6195 * mAmps.to_f - 2.4647 - zero.to_f
	end
	
	# convert PSI to mL/min (venturi meter)
	def flow(diff, rho, cd)
	  # convert to mL/min
	  if diff < 0
	    return 0
      end
	  return cd.to_f * sqrt((2.0*(diff.to_f * @psi2pa.to_f)/rho.to_f))*@a1.to_f/(sqrt((@a1.to_f/@a2.to_f)**2.0-1.0)) * 60.0 * 1000000.0
	end

    # Convert to configuration file string
    def to_config
      "  PROCESSOR #{@name} #{self.class.name.to_s.class_name_to_filename} #{@item_name} #{@value_type}\n"
    end

  end # class PressureProcessor

end # module Cosmos
