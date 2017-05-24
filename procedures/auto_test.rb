###############
# Automated test for the Pump and patch heaters
###############

##TODO: figure out how to control the patch heaters
##		set up more commands for labview assuming we are controling the heaters through it
##		create data packet structure

pSpeed = 0
Temp = 0

# step to next pump speed
def pumpIncrease
	pSpeed = pSpeed + 1
	cmd("PI PUMP with LENGTH 9, CMD_ID 48, VOLTAGE #{pSpeed}")
	Temp = 0
end

#get data from Labview
def Handshake
	cmd("LABVIEW SENDDATA with DATA 1")
end



