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
	  @resolution = resolution.to_f
	  @rho = rho.to_f
	  @psi2pa = 6894.75729
	  @in2m = 0.0254
	  d1 = 0.269	# inches
	  d2 = 0.14 	# inches
	  @a1 = PI*(d1*@in2m)**2/4.0
	  @a2 = PI*(d2*@in2m)**2/4.0
    end

    # Run conversions on the pressure readings
    #
    # See Processor#call
    def call(packet, buffer)
	  @results[:VENTURI_VOLTS] = voltage(packet.read("VENTURI", :RAW, buffer), @resolution)
	  @results[:VENTURI_CURRENT] = milliAmps(@results[:VENTURI_VOLTS])
	  @results[:VENTURI_PRESSURE] = venturiPSI(@results[:VENTURI_CURRENT])
	  @results[:VENTURI_FLOW] = flow(@results[:VENTURI_PRESSURE], @rho, @cd)
	  
	  @results[:STATIC_VOLTS] = voltage(packet.read("STATIC", :RAW, buffer), @resolution)
	  @results[:STATIC_CURRENT] = milliAmps(@results[:STATIC_VOLTS])
	  @results[:STATIC_PRESSURE] = staticPSI(@results[:STATIC_CURRENT])
    end
	
	# convert ADC code to voltage
	def voltage(code, resolution, gain=2.0)
	  max = 2**(resolution.to_f-1)-1
	  return (code.to_f / max.to_f) * (2.048 / gain.to_f) * (180.0 / 33.0)
	end
	
	# convert ADC voltage to milli-amps
	def milliAmps(volts)
	  return volts.to_f / 0.249
	end
	
	# convert mA to PSI (static pressure)
	def staticPSI(mAmps)
	  return 1.8748 * mAmps.to_f - 7.4685
	end
	
	# convert mA to PSI (venturi pressure)
	def venturiPSI(mAmps)
	  return 0 #TODO
	end
	
	# convert volts to inches (level sensor)
	def level(volts)
	  return -1.6912*(volts.to_f)**2.0 + 7.9319 * volts.to_f + 0.2633
	end
	
	# convert mA to inches (venturi meter)
	def flow(diff, rho, cd)
	  # convert to mL/min
	  return cd.to_f * sqrt((2.0*(diff.to_f * @psi2pa.to_f)/rho.to_f))*@a1.to_f/(sqrt((@a1.to_f/@a2.to_f)**2.0-1.0)) * 60.0 * 1000000.0
	end

    # Convert to configuration file string
    def to_config
      "  PROCESSOR #{@name} #{self.class.name.to_s.class_name_to_filename} #{@item_name} #{@value_type}\n"
    end

  end # class PressureProcessor

end # module Cosmos
