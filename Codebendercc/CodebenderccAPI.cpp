/**********************************************************\

  Auto-generated CodebenderccAPI.cpp

\**********************************************************/
#include "CodebenderccAPI.h"

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////deprecated////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FB::variant CodebenderccAPI::download() {
    return "deprecated";
}	

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////public//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool CodebenderccAPI::openPort(const std::string &port, const unsigned int &baudrate) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::openPort",3);
	std::string device;
	device = port;
	#ifdef _WIN32
		device = "\\\\.\\" + port;
	#endif
	if(AddtoPortList(device))
	{ //Check if device is used by someone else
		try
			{
			usedPort=device;		
				if (serialPort.isOpen() == false){
					// need to set a zero timeout when on Windows
					#ifdef _WIN32
						portTimeout = Timeout(std::numeric_limits<uint32_t>::max(), 0, 0, 0, 0);
					#else
						portTimeout = Timeout(std::numeric_limits<uint32_t>::max(), 1000, 0, 1000, 0);
					#endif
					serialPort.setPort(device);				//set port name
					serialPort.setBaudrate(baudrate);		//set port baudrate
					serialPort.setTimeout(portTimeout);		//set the read/write timeout of the port
					serialPort.open();			//open the port
					serialPort.setDTR(true);	//set Data Transfer signal, needed for Arduino leonardo
					serialPort.setRTS(false);	//set Request to Send signal to false, needed for Arduino leonardo  
											}
			}catch(serial::PortNotOpenedException& pno){
				CodebenderccAPI::debugMessage(pno.what(),2);
				perror("Error opening port.");
				RemovePortFromList(usedPort);
				return false;
											}
			catch(serial::SerialException& se){
				CodebenderccAPI::debugMessage(se.what(),2);
				perror("Error opening port.");
				RemovePortFromList(usedPort);
				return false;
									}			
			catch(std::invalid_argument& inv_arg){
				CodebenderccAPI::debugMessage(inv_arg.what(),2);
				perror("Error opening port.");
				RemovePortFromList(usedPort);
				return false;
									}	
			catch(serial::IOException& IOe){
				CodebenderccAPI::debugMessage(IOe.what(),2);
				perror("Error opening port.");
				RemovePortFromList(usedPort);
				return false;
								}						
	}	
	else{
		CodebenderccAPI::debugMessage("CodebenderccAPI::Port is already in use.",3);
		return false;
		}	
	CodebenderccAPI::debugMessage("CodebenderccAPI::openPort ended",3);
	return true;
} catch (...) {
    error_notify("CodebenderccAPI::openPort() threw an unknown exception");
	return false;
}

void CodebenderccAPI::closePort() try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::closePort",3);		
	try{
		if(serialPort.isOpen())
			serialPort.close();
			RemovePortFromList(usedPort);
	}catch(...){
	CodebenderccAPI::debugMessage("CodebenderccAPI::closePort exception",2);
	RemovePortFromList(usedPort);
	}
	CodebenderccAPI::debugMessage("CodebenderccAPI::closePort ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::closePort() threw an unknown exception");
}

#ifdef _WIN32

std::string CodebenderccAPI::probeUSB() try {
	CodebenderccAPI::debugMessageProbe("CodebenderccAPI::probeUSB probing USB ports",3);
	std::string ports ="";
	HKEY hKey;
   /*
    * Open the registry key where serial port key-value pairs are stored.
	*/
   if( CodebenderccAPI::RegOpenKeyEx( HKEY_LOCAL_MACHINE,		// The name of the registry key handle is always the same.
        TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM\\"),	// The same applies to the subkey, since we are looking for serial ports only.
        0,
        KEY_READ,								// Set the access rights, before reading the key contents.
        &hKey) == ERROR_SUCCESS					// Set the variable that revieves the open key handle.
      )
   {
		ports.append(CodebenderccAPI::QueryKey(hKey));  // Call QueryKey function to retrieve the available ports.
      
		if(lastPortCount!=ports.length()){
			lastPortCount=ports.length();
			CodebenderccAPI::detectNewPort(ports);
										}
		CodebenderccAPI::RegCloseKey(hKey);	// Need to close the key handle after the task is completed.
   }
    return ports;
} catch (...) {
    error_notify("[Windows] CodebenderccAPI::probeUSB() threw an unknown exception");
    return "";
}
#else

std::string CodebenderccAPI::probeUSB() try {
	CodebenderccAPI::debugMessageProbe("CodebenderccAPI::probeUSB probing USB ports",3);

    DIR *dp;
    std::string dirs = "";
	struct dirent *ep;
    dp = CodebenderccAPI::opendir("/dev/");
    if (dp != NULL) 
    {
        while (ep = CodebenderccAPI::readdir(dp)) 
        {
            //UNIX ARDUINO PORTS
            if (boost::contains(ep->d_name, "ttyACM") || boost::contains(ep->d_name, "ttyUSB")) {
                dirs += "/dev/";
                dirs += ep->d_name;
                dirs += ",";
            } else if (boost::contains(ep->d_name, "cu.")) {
                dirs += "/dev/";
                dirs += ep->d_name;
                dirs += ",";
            }
        }
        CodebenderccAPI::closedir(dp);
	} else{
		CodebenderccAPI::debugMessage("CodebenderccAPI::probeUSB could not open directory",2);
	}	
	if((size_t)lastPortCount!=dirs.length()){
		lastPortCount=dirs.length();
		CodebenderccAPI::detectNewPort(dirs);
    return dirs;}

    return dirs;


} catch (...) {
    error_notify("CodebenderccAPI::probeUSB() threw an unknown exception");
    return "";
}
#endif

#ifdef _WIN32

int CodebenderccAPI::winExecAvrdude(const std::wstring & command, bool appendFlag) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::winExecAvrdude",3);
	DWORD dwExitCode = -1;
	DWORD APPEND;
	DWORD CREATE;
	if (appendFlag){
		APPEND = FILE_APPEND_DATA;
		CREATE = OPEN_ALWAYS;
	}
	else{
		APPEND = GENERIC_WRITE;
		CREATE = CREATE_ALWAYS;
	}
   
	std::string strResult; // Contains the result of the child process created below.

	BOOL success;

	// Create security attributes to create pipe.
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES)} ;
	sa.bInheritHandle       = TRUE; // Set the bInheritHandle flag so pipe handles are inherited by child process. Required.
	sa.lpSecurityDescriptor = NULL; // Specify a security descriptor. Required.

	STARTUPINFO si = { sizeof(STARTUPINFO) }; // Specify the necessary parameters for child process.
	si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // STARTF_USESTDHANDLES is required.
	si.wShowWindow = SW_HIDE; // Prevent cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

	PROCESS_INFORMATION pi  = { 0 }; // Create an empty process information struct. Needed to get the return value of the command.

	HANDLE fh = CodebenderccAPI::CreateFile(		// Create a file handle pointing to the output file, in order to capture the output.
		&outfile[0], 
		APPEND,
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		&sa,
		CREATE, 
		FILE_FLAG_SEQUENTIAL_SCAN, 
		0);
	// Bind the stdinput, stdoutput and stderror to the output file in order to capture all the output of the command.
	si.hStdOutput = fh;
	si.hStdError = fh; 
	si.hStdInput = fh; 

	// Create the child process. The command simply executes the contents of the batch file, which is the actual command.
	success = CodebenderccAPI::CreateProcess(
		NULL,
		(LPWSTR)command.c_str(),     // command line
		NULL,               // process security attributes
		NULL,               // primary thread security attributes
		TRUE,               // Inherit pipe handles from parent process
		CREATE_NEW_CONSOLE, // creation flags
		NULL,               // use parent's environment
		current_dir,        // use the plugin's directory
		&si,                // __in, STARTUPINFO pointer
		&pi);               // __out, receives PROCESS_INFORMATION

	if (! success)
	{
		CodebenderccAPI::debugMessage("Failed to create child process.", 1);
		return -200;
	}
 
	DWORD dwFileSizeOld=0;
	DWORD dwFileSizeNew=0;
	int counter = 0;

	do {
		CodebenderccAPI::GetExitCodeProcess(pi.hProcess, &dwExitCode);
		// Check if created process is still active.
		if(dwExitCode==STILL_ACTIVE){
			delay(10);
			dwFileSizeNew=GetFileSize( fh, NULL );
			// Check if output file changes.
			if (dwFileSizeOld == dwFileSizeNew)
				counter++;
			else{
				dwFileSizeOld = dwFileSizeNew;
				counter = 0;
			}
		}else
			break;
	}while(counter <= 2000);

	if(dwExitCode == STILL_ACTIVE){
		CodebenderccAPI::TerminateProcess( pi.hProcess, 0 ); // Kill process if it is still running.
		dwExitCode = -204;
	}

	CodebenderccAPI::CloseHandle(fh);
	// CreateProcess docs specify that these must be closed. 
	CodebenderccAPI::CloseHandle( pi.hProcess );
	CodebenderccAPI::CloseHandle( pi.hThread );	
	CodebenderccAPI::debugMessage("CodebenderccAPI::winExecAvrdude ended",3);
	return dwExitCode;
} catch (...) {
    error_notify("CodebenderccAPI::winExecAvrdude() threw an unknown exception");
    return 0;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////PRIVATE////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CodebenderccAPI::doflash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr & flash_callback) try {
    CodebenderccAPI::debugMessage("CodebenderccAPI::doflash",3);

	mtxAvrdudeFlag.lock();
	if(isAvrdudeRunning){
		flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-23));
		mtxAvrdudeFlag.unlock();
		return;
		}
	isAvrdudeRunning=true;
	mtxAvrdudeFlag.unlock();
	std::string os = getPlugin().get()->getOS();

	try {
		if(mcu == "atmega32u4" || AddtoPortList(device))
			{
                #ifndef _WIN32
					chmod(avrdude.c_str(), S_IRWXU);
				#endif
				if (mcu == "atmega32u4") {
					notify(MSG_LEONARD_AUTORESET);
				}
				unsigned char buffer [150000];
				size_t size = base64_decode(code.c_str(), buffer, 150000);
				saveToBin(buffer, size);
				std::string fdevice = device;		
				if (mcu == "atmega32u4") {
					try {	
						// set the "magic" baudrate to force leonardo reset
						if(!CodebenderccAPI::openPort(fdevice, 1200))
						{
						flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-22));
						isAvrdudeRunning=false;	
						return;
						}
						delay(2000);
						CodebenderccAPI::closePort();
						// delay for 300 ms so that the reset is complete
						if ((os == "Windows") || (os == "X11"))
							delay(300);
					}catch (...) {
					CodebenderccAPI::debugMessage("CodebenderccAPI::doflash exception thrown while opening serial port",2);
						perror("Error opening leonardo port");
					RemovePortFromList(fdevice);
					}

					// get the list of ports before resetting the leonardo board
					std::string oldports = probeUSB();
					perror(oldports.c_str());
					std::istringstream oldStream(oldports);
					std::string token;
					std::vector<std::string> oldPorts;

					while(std::getline(oldStream, token, ',')) {
						oldPorts.push_back(token);
					}
					CodebenderccAPI::debugMessage("Listing serial port changes..",2);
					std::string oldPortsMessage = "Ports : {" + oldports + "}";
					CodebenderccAPI::debugMessage(oldPortsMessage.c_str(),2);
					
					// Get the list of ports until the port 
					int elapsed_time = 0;
					bool found = false;
					while(elapsed_time <= 10000){
						std::string newports = probeUSB();
						perror(newports.c_str());
						std::stringstream ss(newports);
						std::string item;
						std::vector<std::string> newPorts;
						while (std::getline(ss, item, ',')) {
							newPorts.push_back(item);
						}
						std::string newPortsMessage = "Ports : {" + newports + "}";
						CodebenderccAPI::debugMessage(newPortsMessage.c_str(),2);
			
						// Check if the new list of ports contains a port that did not exist in the previous list.
						for (std::vector<std::string>::iterator it = newPorts.begin(); it != newPorts.end(); ++it) {
							if (std::find(oldPorts.begin(), oldPorts.end(), *it) == oldPorts.end()){
								fdevice = *it;
								found = true;
								break;
							}
						}

						if (found){		// The new leonardo port appeared in the list. Save it and go on..
								std::string leonardoDeviceMessage = "Found leonardo device on " + fdevice + " port";
								CodebenderccAPI::debugMessage(leonardoDeviceMessage.c_str(),2);
								AddtoPortList(fdevice);
							break;
						}

						oldPorts = newPorts;
						delay(250);
						elapsed_time += 250;

						// If a certain ammount of time has gone by, and the initial port is in the lost of ports, upload using this port
						if(((os == "Widnows" && elapsed_time >= 500) || elapsed_time >=5000) && (std::find(oldPorts.begin(), oldPorts.end(), fdevice) != oldPorts.end()) ){
							std::string uploadingDeviceMessage = "Uploading using selected port: {" + fdevice +"}";
							CodebenderccAPI::debugMessage(uploadingDeviceMessage.c_str(),2);
							break;
						}

						if (elapsed_time == 10000) {
							notify("Could not auto-reset or detect a manual reset!");
							flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-1));
							RemovePortFromList(fdevice);
						}
					}
				}

				int retVal = 1;		
				#ifndef _WIN32
					std::string command = "\"" + avrdude + "\"" + " -C\"" + avrdudeConf + "\"";
				#else
					std::string command = avrdude + " -C" + avrdudeConf;
				#endif
				if (CodebenderccAPI::checkDebug() && currentLevel >= 2){
					command += " -v -v -v -v";
				}else{
					command += " -V";
				}
				command += " -P";
				command += (os == "Windows") ? "\\\\.\\" : "";
				command += fdevice
					+ " -p" + mcu;

				#ifdef _WIN32
					command += " -u -D -U flash:w:file.bin:a";
				#else
					command += " -u -D -U flash:w:\"" + binFile + "\":a";
				#endif
				command += " -c" + protocol
				+ " -b" + speed
				+ " -F";

				retVal = CodebenderccAPI::runAvrdude(command, false);
				_retVal = retVal;
				
				// If the current board is leonardo, wait for a few seconds until the sketch actually takes control of the port
				if (mcu == "atmega32u4"){
					delay(500);
					int timer = 0;
					while(timer < 2000){
						std::vector<std::string> portVector;
						std::string ports = probeUSB();
						std::stringstream chk(ports);
						std::string tok;
						while (std::getline(chk, tok, ',')) {
							portVector.push_back(tok);
						}
						// check if the bootloader has finished and the sketch has taken control of the port
						if (std::find(portVector.begin(), portVector.end(), fdevice) != portVector.end()){
							delay(100);
							timer += 100;
							break;
						}
						delay(100);
						timer += 100;
					}
				}
				RemovePortFromList(fdevice);
				isAvrdudeRunning=false;	
				flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
			}
		else{
				CodebenderccAPI::debugMessage("Port is in use, choose another port",3);
				isAvrdudeRunning=false;	
				flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-22));
			}	
		} 
	catch (...) 
	{
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflash exception",2);
		isAvrdudeRunning=false;	
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
		RemovePortFromList(device);
    }
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflash ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::doFlash() threw an unknown exception");
}

void CodebenderccAPI::doflashWithProgrammer(const std::string& device, const std::string& code, const std::string& maxsize, std::map<std::string, std::string>& programmerData, const std::string& mcu, const FB::JSObjectPtr & flash_callback) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer",3);
	mtxAvrdudeFlag.lock();
	if(isAvrdudeRunning){
		flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-23));
		mtxAvrdudeFlag.unlock();
		return;
		}
	isAvrdudeRunning=true;
	mtxAvrdudeFlag.unlock();

	std::string os = getPlugin().get()->getOS();
	
	try {
			if((programmerData["communication"] == "usb")||(programmerData["communication"] == "")||(AddtoPortList(device)))
			{
				#ifndef _WIN32
						chmod(avrdude.c_str(), S_IRWXU);
				#endif

				unsigned char buffer [150000];
				size_t size = base64_decode(code.c_str(), buffer, 150000);
				saveToBin(buffer, size);
				
				std::string fdevice = device;
				int retVal = 1;

				// Create the first part of the command, which includes the programmer settings.
				programmerData["mcu"] = mcu.c_str();
				programmerData["device"] = fdevice.c_str();
				std::string command = setProgrammerCommand(programmerData);

				#ifdef _WIN32
					command += " -Uflash:w:file.bin:a";
				#else
					command += " -Uflash:w:\"" + binFile + "\":a";
				#endif
				// Execute the upload command.
			
				retVal = CodebenderccAPI::runAvrdude(command, false);
				_retVal = retVal;
				flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
				RemovePortFromList(device);
			}
			else
			{
				flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-22));
				isAvrdudeRunning=false;	
				CodebenderccAPI::debugMessage("CodebenderccAPI::Port is already in use.",3);
			}
	}catch(...){
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer exception",2);
		isAvrdudeRunning=false;	
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
		RemovePortFromList(device);
    }
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer ended",3);
	isAvrdudeRunning=false;	
} catch (...) {
    error_notify("CodebenderccAPI::doflashWithProgrammer() threw an unknown exception");
}

void CodebenderccAPI::doflashBootloader(const std::string& device,  std::map<std::string, std::string>& programmerData, std::map<std::string, std::string>& bootloaderData, const std::string& mcu, const FB::JSObjectPtr & flash_callback) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader",3);
	mtxAvrdudeFlag.lock();
	if(isAvrdudeRunning){
		flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-23));
		mtxAvrdudeFlag.unlock();
		return;
		}
	isAvrdudeRunning=true;
	mtxAvrdudeFlag.unlock();
	std::string os = getPlugin().get()->getOS();
	try {
		if((programmerData["communication"] == "usb")||(programmerData["communication"] == "")||(AddtoPortList(device)))
		{
			#ifndef _WIN32
				chmod(avrdude.c_str(), S_IRWXU);
			#endif		
			
			std::string fdevice = device;
			int retVal = 1;

			// Create the first part of the command, which includes the programmer settings.
			programmerData["mcu"] = mcu.c_str();
			programmerData["device"] = fdevice.c_str();
			std::string programmerCommand = setProgrammerCommand(programmerData);

			// The first part of the command is very likely to be used again when flashing the hex file.
			std::string command = programmerCommand;

			/**
			  * Erase the chip, applying the proper values to the unlock bits and high/low/extended fuses.
			  * Note: Values for high and low fuses MUST exist. The other values are optional, depending on the chip.
			  */
			command += " -e";
			command += (bootloaderData["ulbits"] != "") ? " -Ulock:w:" + bootloaderData["ulbits"] + ":m" : "";
			command += (bootloaderData["efuses"] != "") ? " -Uefuse:w:" + bootloaderData["efuses"] + ":m" : "";
			command += " -Uhfuse:w:" + bootloaderData["hfuses"] + ":m"
					+ " -Ulfuse:w:" + bootloaderData["lfuses"] + ":m";
			
			
			retVal = CodebenderccAPI::runAvrdude(command, false);
			_retVal = retVal;

			// If avrdude failed return the error code, else continue.
			if (retVal == 0){

				// Apply a delay of one second.
				delay(1000);
		
				// Check if hex bootloader wsa sent from the server. If no bootloader exists,
				// an empty file is created.
				std::ifstream file (hexFile.c_str(), std::ifstream::binary);
				if (file.is_open()){
					file.seekg (0, file.end);
					int length = file.tellg();

					if (length == 0){
						file.close();
					}else{
						command = programmerCommand;
						#ifdef _WIN32
							command += " -Uflash:w:bootloader.hex:i";
						#else
							command += " -Uflash:w:\"" + hexFile + "\":i";
						#endif
						command += (bootloaderData["lbits"] != "") ? " -Ulock:w:" + bootloaderData["lbits"] + ":m" : "";

						
						retVal = CodebenderccAPI::runAvrdude(command, true);
						_retVal = retVal;
					}
				}
			}
			flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
			RemovePortFromList(fdevice);
		}
	else
		{
			flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-22));
			isAvrdudeRunning=false;	
			CodebenderccAPI::debugMessage("CodebenderccAPI::Port is already in use:",3);
		}	
	}catch(...){
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader exception",2);
		isAvrdudeRunning=false;	
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
		RemovePortFromList(device);
	}
	isAvrdudeRunning=false;	
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::doflashBootloader() threw an unknown exception");
}

const std::string CodebenderccAPI::setProgrammerCommand(std::map<std::string, std::string>& programmerData) try {
	
	CodebenderccAPI::debugMessage("CodebenderccAPI::setProgrammerCommand",3);
	std::string os = getPlugin().get()->getOS();
	#ifndef _WIN32
			std::string command = "\"" + avrdude + "\"" + " -C\"" + avrdudeConf + "\"";
	#else
			std::string command = avrdude + " -C" + avrdudeConf;
	#endif
	/**
	* Check if debugging is set to true. If the debug verbosity level is greater than 1,
    * add verbosity flags to the avrdude command.
	*/
	if (CodebenderccAPI::checkDebug() && currentLevel >= 2){
		command += " -v -v -v -v";
	}else{
		command += " -V";
	}
	command += " -p" + programmerData["mcu"] + " -c" + programmerData["protocol"];
	if (programmerData["communication"] == "usb"){
		command += " -Pusb";
	}else if (programmerData["communication"] == "serial"){
		command += " -P";
		command += (os == "Windows") ? "\\\\.\\" : "";
		command += programmerData["device"];
		if (programmerData["speed"] != "0"){
			command += " -b" + programmerData["speed"];
		}
	}
	if (programmerData["force"] == "true")
		command += " -F";
	if (programmerData["delay"] != "0")
		command += " -i" + programmerData["delay"];

	CodebenderccAPI::debugMessage("CodebenderccAPI::setProgrammerCommand ended",3);
	return command;
} catch (...) {
    error_notify("CodebenderccAPI::setProgrammerCommand() threw an unknown exception");
    return "";
}

int CodebenderccAPI::runAvrdude(const std::string& command, bool append) try {

	CodebenderccAPI::debugMessage("CodebenderccAPI::runAvrdude",3);
	int retval = 1;
	#ifdef _WIN32
		// Ιf on Windows, create a batch file and save the command in that file.
		std::ofstream batchFd; 
		try{
			batchFd.open(batchFile.c_str());
			batchFd << command;
			batchFd.close();
		}catch(...){
			CodebenderccAPI::debugMessage("Failed to write command to batch file!",1);
		}		
		lastcommand = command;
		// Call winExecAvrdude, which creates a new process, runs the batch file and gets all the ouput.
		retval = winExecAvrdude(batchFile, append);
	#else
		/**
		 * If on Unix-like system, call unixExecAvrdude which makes a duplicate of the current process (fork) 
		 * and replaces the entire new process with avrdude (exec).
		 */
		lastcommand = command;
		
		retval = unixExecAvrdude(command, append);
	#endif
		/** Print the content of the output file, if debugging is on. **/
	if (CodebenderccAPI::checkDebug())
	{
		std::ifstream ifs(outfile.c_str());
		std::string content( (std::istreambuf_iterator<char>(ifs) ),
		(std::istreambuf_iterator<char>()    ) );
		CodebenderccAPI::debugMessage(content.c_str(),1);
		}
	CodebenderccAPI::debugMessage(lastcommand.c_str(),1);
	CodebenderccAPI::debugMessage("CodebenderccAPI::runAvrdude ended",3);
	return retval;
} catch (...) {
    error_notify("CodebenderccAPI::runAvrdude() threw an unknown exception");
    return 1;
}

#if !defined  _WIN32
int CodebenderccAPI::unixExecAvrdude (const std::string &command, bool appendFlag) try{
	std::string executionCommand = command;
	/* Convert string vector to char array */
	std::vector<char *> cmd_argv(4);
	cmd_argv[0] = "sh";
	cmd_argv[1] = "-c";
	cmd_argv[2] = &executionCommand[0];
	cmd_argv[3] = NULL;

    pid_t pid, w;

    /* make a duplicate of the current process */
    pid = CodebenderccAPI::fork();
    if (pid == -1)
        return -200; // fork() failed

    if (pid == 0) {
        pid_t cpid = getpid();
        setpgid(cpid, cpid);

    	const char *frpathout = outfile.c_str();

        stdout = CodebenderccAPI::freopen(frpathout,
                                          appendFlag ? "a" : "w",
                                          stdout);
        stderr = CodebenderccAPI::freopen(frpathout,
                                          appendFlag ? "a" : "w",
                                          stderr);

        CodebenderccAPI::execvp(cmd_argv[0], cmd_argv.data());
        _exit(EXIT_FAILURE);
    }

    long oldSize=0;
    long newSize=0;
    int counter =0;

    do {
        int status = 0;

        w = CodebenderccAPI::waitpid(pid, &status, WNOHANG);
        if (w == -1)
            return -202; // waitpid() failed

        if (w == 0) {
		/* wait for 0.0001 sec before getting the size of the output file*/
		delay(1);
		/* Check if the file exists */
		boost::filesystem::path out(outfile);
	 		if(boost::filesystem::exists(out))
	 			{
				/* the child's state has not changed */
				newSize = CodebenderccAPI::filesize(outfile.c_str());
		            if (newSize == -1)
		                break;
		            if (newSize == oldSize)
		                counter++;
		            else
					{
		                oldSize = newSize;
						counter = 0;
					}
				}
				/* If file doesn't exist, increase counter */
				else 
				{
				counter++;	
				}	
        }
        else if(WIFSIGNALED(status))
        {
            return -203;
        }
        else if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
    } while (counter != 20000);

    killpg(pid, SIGKILL);
	
    return -204; // child process was killed
} catch (...){
	error_notify("CodebenderccAPI::unixExecAvrdude() threw an unknown exception");
	return -205;
}
#endif

#if !defined  _WIN32
long CodebenderccAPI::filesize(const char *filename) try{
	struct stat buf;

    if (CodebenderccAPI::stat(filename, &buf) == -1)
        return -1;

	return buf.st_size;
} catch (...){
	error_notify("CodebenderccAPI::filesize() threw an unknown exception");
	return -1;
}
#endif

/**
 * Save the binary data to the binary file specified in the constructor.
 */
void CodebenderccAPI::saveToBin(unsigned char * data, size_t size) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::saveToBin",3);
#if !defined _WIN32 ||_WIN34
    FILE *fp = CodebenderccAPI::fopen(binFile.c_str(), "wb");
#else
	const std::string stbinFile(binFile.begin(),binFile.end());
	FILE *fp = CodebenderccAPI::fopen(stbinFile.c_str(), "wb");
#endif
    if (!fp)
        return;

    CodebenderccAPI::fwrite(data, size, 1, fp);
    CodebenderccAPI::fclose(fp);

	CodebenderccAPI::debugMessage("CodebenderccAPI::saveToBin ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::saveToBin() threw an unknown exception");
}

/**
 * Save the hex data of the bootloader file to the hex file specified in the constructor.
 */
void CodebenderccAPI::saveToHex(const std::string& hexContent) try {
    CodebenderccAPI::debugMessage("CodebenderccAPI::saveToHex",3);
#if !defined _WIN32
    FILE *fp = CodebenderccAPI::fopen(hexFile.c_str(), "wb");
#else
	const std::string sthexFile(hexFile.begin(),hexFile.end());
	FILE *fp = CodebenderccAPI::fopen(sthexFile.c_str(), "wb");
#endif
    if (!fp)
        return;

    CodebenderccAPI::fwrite(hexContent.c_str(), hexContent.length(), hexContent.length() ? 1 : 0, fp);
    CodebenderccAPI::fclose(fp);

    CodebenderccAPI::debugMessage("CodebenderccAPI::saveToHex ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::saveToHex() threw an unknown exception");
}

void CodebenderccAPI::detectNewPort(const std::string& portString) try {
	
	std::vector<std::string> portVector;
	std::vector< std::string >::iterator externalIterator = portsList.begin();

	std::string mess;

	std::stringstream chk(portString);
	std::string tok;
	while (std::getline(chk, tok, ',')) {
		portVector.push_back(tok);
	}
	
	std::vector< std::string >::iterator internalIterator = portVector.begin();
	
	while (externalIterator != portsList.end()){
		if (std::find(portVector.begin(), portVector.end(), *externalIterator) == portVector.end()){
			mess = "Device removed from port : " + *externalIterator;
			CodebenderccAPI::debugMessage(mess.c_str(),1);
		}
		externalIterator++;
	}
	while (internalIterator != portVector.end()){
		if (std::find(portsList.begin(), portsList.end(), *internalIterator) == portsList.end()){
			mess = "Device added to port : " + *internalIterator;
			CodebenderccAPI::debugMessage(mess.c_str(),1);
		}
		internalIterator++;
	}
	
	portsList = portVector;
} catch (...) {
    error_notify("CodebenderccAPI::detectNewPort() threw an unknown exception");
}

void CodebenderccAPI::serialReader(const std::string &port, const unsigned int &baudrate, const FB::JSObjectPtr & callback) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::serialReader",3);	
	if(!CodebenderccAPI::openPort(port, baudrate))
		{
		notify("disconnect");
		return;
		}
	try {
		int d;
		std::string rcvd;		
		for (;;) {
			if(serialPort.isOpen())
				rcvd = "";	
				rcvd = serialPort.read((size_t) 1);
					if(rcvd != ""){
						d = (int) rcvd[0];
						std::string characterMessage="Received character:";
						characterMessage.append(&rcvd[0]);
						CodebenderccAPI::debugMessage(characterMessage.c_str(),2);
						callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(d));
					}
			}
		}	
    catch (...) {
	CodebenderccAPI::debugMessage("CodebenderccAPI::serialReader loop interrupted",1);
		closePort();
		/*Port is already closed from closePort() and notify("disconnect") closes the port for second time*/
        notify("disconnect");
    }
	CodebenderccAPI::debugMessage("CodebenderccAPI::serialReader ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::serialReader() threw an unknown exception");
}

std::string CodebenderccAPI::exec(const char * cmd) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::exec",3);
	std::string result = "";
#ifndef _WIN32
    FILE* pipe = CodebenderccAPI::popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    while (CodebenderccAPI::fgets(buffer, 128, pipe) != NULL) {

        result += buffer;
    }
    CodebenderccAPI::pclose(pipe);
#endif
	CodebenderccAPI::debugMessage("CodebenderccAPI::exec ended",3);
    return result;
} catch (...) {
    error_notify("CodebenderccAPI::exec() threw an unknown exception");
    return "";
}


void CodebenderccAPI::notify(const std::string &message) {
CodebenderccAPI::debugMessage("CodebenderccAPI::notify",3);	
callback_->InvokeAsync("", FB::variant_list_of(shared_from_this())(message.c_str()));
}

void CodebenderccAPI::error_notify(const std::string &message) {
	CodebenderccAPI::debugMessage("CodebenderccAPI::error_notify",3);	
	error_callback_->InvokeAsync("", FB::variant_list_of(shared_from_this())(message.c_str()));
}

DIR *
CodebenderccAPI::opendir(const char *name)
{
    DIR *dp;

    dp = ::opendir(name);
    if (dp)
        return dp;

    std::string err_msg = "CodebenderccAPI::opendir() - ";

    switch (errno) {
        case EACCES:
            err_msg += "EACCES: Permission denied.";
            break;
        case EMFILE:
            err_msg += "EMFILE: Too many file descriptors in use by process.";
            break;
        case ENFILE:
            err_msg += "ENFILE: Too many files are currently open in the system.";
            break;
        case ENOENT:
            err_msg += "ENOENT: Directory does not exist, or name is an empty string.";
            break;
        case ENOMEM:
            err_msg += "ENOMEM: Insufficient memory to complete the operation.";
            break;
        case ENOTDIR:
            err_msg += "ENOTDIR: name is not a directory.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return NULL;
}

struct dirent *
CodebenderccAPI::readdir(DIR *dirp)
{
    struct dirent *dent;

    errno = 0;
    dent = ::readdir(dirp);

    if (errno == 0)
        return dent;

    std::string err_msg = "CodebenderccAPI::readdir() - ";

    switch (errno) {
        case EBADF:
            err_msg += "EBADF: Invalid directory stream descriptor dirp.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return NULL;
}

void
CodebenderccAPI::closedir(DIR *dirp)
{
    int rc;

    rc = ::closedir(dirp);
    if (rc == 0)
        return;

    std::string err_msg = "CodebenderccAPI::closedir() - ";

    switch (errno) {
        case EBADF:
            err_msg += "EBADF: Invalid directory stream descriptor dirp.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
}

FILE *
CodebenderccAPI::fopen(const char *path, const char *mode)
{
    FILE *fp;

    fp = ::fopen(path, mode);
    if (fp)
        return fp;

    std::string err_msg = "CodebenderccAPI::fopen() - ";

    switch (errno) {
        case EINVAL:
            err_msg += "EINVAL: The mode provided to fopen() was invalid.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return NULL;
}

FILE *
CodebenderccAPI::freopen(const char *path, const char *mode, FILE *stream)
{
    FILE *fp;

    fp = ::freopen(path, mode, stream);
    if (fp != NULL)
        return fp;

    std::string err_msg = "CodebenderccAPI::freopen() - ";

    switch (errno) {
        case EINVAL:
            err_msg += "EINVAL: The mode provided to freopen() was invalid.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return NULL;
}

size_t
CodebenderccAPI::fwrite(const void *ptr,
                        size_t size,
                        size_t nmemb,
                        FILE *stream)
{
    std::string err_msg = "CodebenderccAPI::fwrite() - ";
    size_t n;

    clearerr(stream);

    n = ::fwrite(ptr, size, nmemb, stream);
    if (n == nmemb)
        return n;

    err_msg += (ferror(stream) != 0) ? "errno set" : "bad I/O";
    error_notify(err_msg);
    return n;
}

char *
CodebenderccAPI::fgets(char *s, int size, FILE *stream)
{
    char *ret;

    clearerr(stream);
    ret = ::fgets(s, size, stream);

    if (ferror(stream)  == 0)
        return ret;

    std::string err_msg = "CodebenderccAPI::fgets() failed";

    error_notify(err_msg);
    return NULL;
}

void
CodebenderccAPI::fclose(FILE *fp)
{
    if (::fclose(fp) == 0)
        return;

    std::string err_msg = "CodebenderccAPI::fclose() - ";

    switch (errno) {
        case EBADF:
            err_msg += "EBADF  The file descriptor underlying fp is not valid.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
}

#ifndef _WIN32
FILE *
CodebenderccAPI::popen(const char *command, const char *type)
{
    FILE *fp;

    fp = ::popen(command, type);
    if (fp)
        return fp;

    std::string err_msg = "CodebenderccAPI::popen() - ";

    switch (errno) {
        case EINVAL:
            err_msg += "EINVAL: invalid type argument";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return NULL;
}

void
CodebenderccAPI::pclose(FILE *stream)
{
    if (::pclose(stream) != -1)
        return;

    std::string err_msg = "CodebenderccAPI::pclose() - ";

    switch (errno) {
        case ECHILD:
            err_msg += "ECHILD: could not obtain the child status";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
}

int
CodebenderccAPI::stat(const char *path, struct stat *buf)
{
    int rc;

    rc = ::stat(path, buf);
    if (rc == 0)
        return rc;

    std::string err_msg = "CodebenderccAPI::stat() - ";

    switch (errno) {
        case EACCES:
            err_msg += "EACCES: Search permission is denied for one of the " \
                       "directories in the path prefix of path.";
            break;
        case EFAULT:
            err_msg += "EFAULT: Bad address";
            break;
        case ELOOP:
            err_msg += "ELOOP: Too many symbolic links encountered while traversing the path";
            break;
        case ENAMETOOLONG:
            err_msg += "ENAMETOOLONG: path is too long";
            break;
        case ENOENT:
            err_msg += "ENOENT: A component of path does not exist, or path is an empty string";
            break;
        case ENOMEM:
            err_msg += "ENOMEM: Out of kernel memory";
            break;
        case ENOTDIR:
            err_msg += "ENOTDIR: A component of the path prefix of path is not a directory.";
            break;
        case EOVERFLOW:
            err_msg += "EOVERFLOW: path or fd refers to a file whose "                          \
                       "size, inode number, or number of blocks cannot  be  represented  in, "  \
                       "respectively, the types off_t, ino_t, or blkcnt_t.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return -1;
}

pid_t
CodebenderccAPI::fork(void)
{
    pid_t pid;

    pid = ::fork();
    if (pid != -1)
        return pid;

    std::string err_msg = "CodebenderccAPI::fork() - ";

    switch (errno) {
        case EAGAIN:
            err_msg += "EAGAIN: cannot allocate sufficient memory to copy the " \
                       "parent's page tables and allocate a  task  structure for the child.";
            break;

        case ENOMEM:
            err_msg += "ENOMEM: failed to allocate the necessary kernel structures" \
                       " because memory is tight.";
            break;

        case ENOSYS:
            err_msg += "ENOSYS: fork() is not supported on this platform.";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return pid;
}

int
CodebenderccAPI::execvp(const char *file, char *const argv[])
{
    int rc;

    rc = ::execvp(file, argv);

    std::string err_msg = "CodebenderccAPI::execvp() - ";

    if (rc == -1)
        err_msg += "Unknown error!";
    else
        err_msg += "execv() returned != -1!!!";

    error_notify(err_msg);
    return rc;
}

pid_t
CodebenderccAPI::waitpid(pid_t pid, int *status, int options)
{
    pid_t cpid;

    cpid = ::waitpid(pid, status, options);
    if (cpid != -1)
        return cpid;

    std::string err_msg = "CodebenderccAPI::waitpid() - ";

    switch (errno) {
        case ECHILD:
            err_msg += "ECHILD: The process specified by pid does not exist or " \
                       "is not a child of the calling process.";
            break;

        case EINTR:
            err_msg += "EINTR: WNOHANG was not set and an unblocked signal or " \
                       "a SIGCHLD was caught; see signal(7).";
            break;

        case EINVAL:
            err_msg += "EINVAL: The option argument was invalid";
            break;

        default:
            err_msg += "Unknown error!";
    }

    error_notify(err_msg);
    return cpid;
}

#endif

/** TODO: we should return the result of WEXITSTATUS(rc) */
int
CodebenderccAPI::system(const char *command)
{
    std::string err_msg = "CodebenderccAPI::system() - ";
    int rc;

    rc = ::system(NULL);
    if (rc == 0)
        error_notify(err_msg + "can not access the shell");

    rc = ::system(command);
    if (rc == -1)
        err_msg += "Unknown error";
#ifndef _WIN32
    else if (WEXITSTATUS(rc) == 127)
        err_msg += "/bin/sh could not be executed";
#endif
    else
        return rc;

    error_notify(err_msg);
    return rc;
}

#ifdef _WIN32
LONG
CodebenderccAPI::RegQueryInfoKey(HKEY hKey,
                                 LPTSTR lpClass,
                                 LPDWORD lpcClass,
                                 LPDWORD lpReserved,
                                 LPDWORD lpcSubKeys,
                                 LPDWORD lpcMaxSubKeyLen,
                                 LPDWORD lpcMaxClassLen,
                                 LPDWORD lpcValues,
                                 LPDWORD lpcMaxValueNameLen,
                                 LPDWORD lpcMaxValueLen,
                                 LPDWORD lpcbSecurityDescriptor,
                                 PFILETIME lpftLastWriteTime)
{
    LONG rc;

    rc = ::RegQueryInfoKey(hKey,
                           lpClass,
                           lpcClass,
                           lpReserved,
                           lpcSubKeys,
                           lpcMaxSubKeyLen,
                           lpcMaxClassLen,
                           lpcValues,
                           lpcMaxValueNameLen,
                           lpcMaxValueLen,
                           lpcbSecurityDescriptor,
                           lpftLastWriteTime);
    if (rc == ERROR_SUCCESS)
        return rc;

    std::string err_msg = "CodebenderccAPI::RegQueryInfoKey() - ";

    switch (rc) {
        case ERROR_MORE_DATA:
            err_msg += "lpClass buffer is too small to receive the name of the class";
            break;

        default:
            err_msg += "System error code: ";
            err_msg += boost::lexical_cast<std::string>(rc);
    }

    error_notify(err_msg);
    return rc;
}

LONG
CodebenderccAPI::RegEnumValue(HKEY hKey,
                              DWORD dwIndex,
                              LPTSTR lpValueName,
                              LPDWORD lpcchValueName,
                              LPDWORD lpReserved,
                              LPDWORD lpType,
                              LPBYTE lpData,
                              LPDWORD lpcbData)
{
    LONG rc;

    rc = ::RegEnumValue(hKey,
                        dwIndex,
                        lpValueName,
                        lpcchValueName,
                        lpReserved,
                        lpType,
                        lpData,
                        lpcbData);
    if (rc == ERROR_SUCCESS)
        return rc;

    std::string err_msg = "CodebenderccAPI::RegEnumValue() - ";

    switch (rc) {
        case ERROR_MORE_DATA:
            err_msg += "lpData buffer is too small to receive the value";
            break;

        case ERROR_NO_MORE_ITEMS:
            err_msg += "There are no more values available";
            break;

        default:
            err_msg += "System error code: ";
            err_msg += boost::lexical_cast<std::string>(rc);
    }

    error_notify(err_msg);
    return rc;
}

LONG
CodebenderccAPI::RegQueryValueEx(HKEY hKey,
                                 LPCTSTR lpValueName,
                                 LPDWORD lpReserved,
                                 LPDWORD lpType,
                                 LPBYTE lpData,
                                 LPDWORD lpcbData)
{
    LONG rc;

    rc = ::RegQueryValueEx(hKey,
                           lpValueName,
                           lpReserved,
                           lpType,
                           lpData,
                           lpcbData);
    if (rc == ERROR_SUCCESS)
        return rc;

    std::string err_msg = "CodebenderccAPI::RegQueryValueEx() - ";

    switch (rc) {
        case ERROR_MORE_DATA:
            err_msg += "lpData buffer is too small to receive the data";
            break;

        case ERROR_FILE_NOT_FOUND:
            err_msg += "lpValueName registry value does not exist";
            break;

        default:
            err_msg += "System error code: ";
            err_msg += boost::lexical_cast<std::string>(rc);
    }

    error_notify(err_msg);
    return rc;
}


LONG
CodebenderccAPI::RegOpenKeyEx(HKEY hKey,
                              LPCTSTR lpSubKey,
                              DWORD ulOptions,
                              REGSAM samDesired,
                              PHKEY phkResult)
{
    LONG rc;
	static bool errorFlag;

    rc = ::RegOpenKeyEx(hKey,
                        lpSubKey,
                        ulOptions,
                        samDesired,
                        phkResult);
	if (rc == ERROR_SUCCESS){
        errorFlag = false;
		return rc;
	}

	if (rc != ERROR_SUCCESS && errorFlag == false){
		errorFlag = true;
		/* TODO: Print error code description with FormatMessage.
	 *      *
	 *           * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724897%28v=vs.85%29.aspx
	 *               */
		std::string err_msg = "CodebenderccAPI::RegOpenKeyEx() - Winerror.h error code: ";
		err_msg += boost::lexical_cast<std::string>(rc);

		error_notify(err_msg);
		return rc;
	}
	
	return rc;
}

LONG
CodebenderccAPI::RegCloseKey(HKEY hKey)
{
    LONG rc;
	static bool errorFlag;

    rc = ::RegCloseKey(hKey);
	if (rc == ERROR_SUCCESS){
		errorFlag = false;	
		return rc;
	}

	if (rc != ERROR_SUCCESS && errorFlag == false){
		errorFlag = true;
		/* TODO: Print error code description with FormatMessage.
	 *      *
	 *           * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724837%28v=vs.85%29.aspx
	 *               */
		std::string err_msg = "CodebenderccAPI::RegCloseKey() - Winerror.h error code: ";
		err_msg += boost::lexical_cast<std::string>(rc);

		error_notify(err_msg);
		return rc;
	}

	return rc;
}

HANDLE
CodebenderccAPI::CreateFile(LPCTSTR lpFileName,
                            DWORD dwDesiredAccess,
                            DWORD dwShareMode,
                            LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            DWORD dwCreationDisposition,
                            DWORD dwFlagsAndAttributes,
                            HANDLE hTemplateFile)
{
    HANDLE rc;

    rc = ::CreateFile(lpFileName,
                      dwDesiredAccess,
                      dwShareMode,
                      lpSecurityAttributes,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      hTemplateFile);
    if (rc != INVALID_HANDLE_VALUE)
        return rc;

    std::string err_msg = "CodebenderccAPI::CreateFile() - extended error information: ";
    err_msg += boost::lexical_cast<std::string>(GetLastError());

    error_notify(err_msg);
    return rc;
}

BOOL
CodebenderccAPI::CreateProcess(LPCTSTR lpApplicationName,
                               LPTSTR lpCommandLine,
                               LPSECURITY_ATTRIBUTES lpProcessAttributes,
                               LPSECURITY_ATTRIBUTES lpThreadAttributes,
                               BOOL bInheritHandles,
                               DWORD dwCreationFlags,
                               LPVOID lpEnvironment,
                               LPCTSTR lpCurrentDirectory,
                               LPSTARTUPINFO lpStartupInfo,
                               LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL rc;

    rc = ::CreateProcess(lpApplicationName,
                         lpCommandLine,
                         lpProcessAttributes,
                         lpThreadAttributes,
                         bInheritHandles,
                         dwCreationFlags,
                         lpEnvironment,
                         lpCurrentDirectory,
                         lpStartupInfo,
                         lpProcessInformation);
    if (rc != 0)
        return rc;

    std::string err_msg = "CodebenderccAPI::CreateProcess() - extended error information: ";
    err_msg += boost::lexical_cast<std::string>(GetLastError());

    error_notify(err_msg);
    return rc;
}

DWORD
CodebenderccAPI::WaitForSingleObject(HANDLE hHandle,
                                     DWORD dwMilliseconds)
{
    DWORD rc;

    rc = ::WaitForSingleObject(hHandle,
                               dwMilliseconds);
    if (rc != WAIT_FAILED)
        return rc;

    std::string err_msg = "CodebenderccAPI::WaitForSingleObject() - extended error information: ";
    err_msg += boost::lexical_cast<std::string>(GetLastError());

    error_notify(err_msg);
    return rc;
}

BOOL
CodebenderccAPI::GetExitCodeProcess(HANDLE hProcess,
                                    LPDWORD lpExitCode)
{
    BOOL rc;

    rc = ::GetExitCodeProcess(hProcess,
                              lpExitCode);
    if (rc != 0)
        return rc;

    std::string err_msg = "CodebenderccAPI::GetExitCodeProcess() - extended error information: ";
    err_msg += boost::lexical_cast<std::string>(GetLastError());

    error_notify(err_msg);
    return rc;
}

BOOL
CodebenderccAPI::TerminateProcess(HANDLE hProcess,
                                  UINT uExitCode)
{
    BOOL rc;

    rc = ::TerminateProcess(hProcess,
                            uExitCode);
    if (rc != 0)
        return rc;

    std::string err_msg = "CodebenderccAPI::TerminateProcess() - extended error information: ";
    err_msg += boost::lexical_cast<std::string>(GetLastError());

    error_notify(err_msg);
    return rc;
}

BOOL
CodebenderccAPI::CloseHandle(HANDLE hObject)
{
    BOOL rc;

    rc = ::CloseHandle(hObject);
    if (rc != 0)
        return rc;

    std::string err_msg = "CodebenderccAPI::CloseHandle() - extended error information: ";
    err_msg += boost::lexical_cast<std::string>(GetLastError());

    error_notify(err_msg);
    return rc;
}

#endif
