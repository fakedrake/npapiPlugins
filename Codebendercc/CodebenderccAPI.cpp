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
	#if defined _WIN32||_WIN64
		device = "\\\\.\\" + port;
	#endif
	if(AddtoPortList(device))
	{ //Check if device is used by someone else
		try
			{
			usedPort=device;		
				if (serialPort.isOpen() == false){
					// need to set a zero timeout when on Windows
					#if defined _WIN32||_WIN64
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
				CodebenderccAPI::debugMessage("CodebenderccAPI::PortNotOpenedException exception",2);
				m_host->htmlLog (pno.what());
				perror("Error opening port.");
				RemovePortFromList(usedPort);
				return false;
											}
			catch(serial::SerialException& se){
				CodebenderccAPI::debugMessage("CodebenderccAPI::SerialException exception",2);
				m_host->htmlLog (se.what());
				perror("Error opening port.");
				RemovePortFromList(usedPort);
				return false;
									}			
			catch(std::invalid_argument& inv_arg){
				CodebenderccAPI::debugMessage("CodebenderccAPI::invalid_argument exception",2);
				m_host->htmlLog (inv_arg.what());
				perror("Error opening port.");
				RemovePortFromList(usedPort);
				return false;
									}	
			catch(serial::IOException& IOe){
				CodebenderccAPI::debugMessage("CodebenderccAPI::IOException exception",2);
				m_host->htmlLog (IOe.what());
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

#if defined _WIN32||_WIN64

std::string CodebenderccAPI::probeUSB() try {
	CodebenderccAPI::debugMessageProbe("CodebenderccAPI::probeUSB probing USB ports",3);
	std::string ports ="";
	HKEY hKey;
   /*
    * Open the registry key where serial port key-value pairs are stored.
	*/
   if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,		// The name of the registry key handle is always the same.
        TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM\\"),	// The same applies to the subkey, since we are looking for serial ports only.
        0,
        KEY_READ,								// Set the access rights, before reading the key contents.
        &hKey) == ERROR_SUCCESS					// Set the variable that revieves the open key handle.
      )
   {
      ports.append(CodebenderccAPI::QueryKey(hKey));  // Call QueryKey function to retrieve the available ports.
   }   
	if(lastPortCount!=ports.length()){
		lastPortCount=ports.length();
		CodebenderccAPI::detectNewPort(ports);
									}
	RegCloseKey(hKey);	// Need to close the key handle after the task is completed.	
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
    dp = opendir("/dev/");
    if (dp != NULL) 
    {
        while (ep = readdir(dp)) 
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
        (void) closedir(dp);
	} else{
		CodebenderccAPI::debugMessage("CodebenderccAPI::probeUSB could not open directory",2);
	}	
	if(lastPortCount!=dirs.length()){
		lastPortCount=dirs.length();
		CodebenderccAPI::detectNewPort(dirs);
    return dirs;}

    return dirs;


} catch (...) {
    error_notify("CodebenderccAPI::probeUSB() threw an unknown exception");
    return "";
}
#endif

#if defined _WIN32 || _WIN64

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

	HANDLE fh = CreateFile(		// Create a file handle pointing to the output file, in order to capture the output.
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
	success = CreateProcess(
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
		return -3;
	}
 
	// Wait until child processes exit. Don't wait forever.
	WaitForSingleObject( pi.hProcess, INFINITE );
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	TerminateProcess( pi.hProcess, 0 ); // Kill process if it is still running
	 
	CloseHandle(fh);
	// CreateProcess docs specify that these must be closed. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );	
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

	std::string os = getPlugin().get()->getOS();
	CodebenderccAPI::getThreadId("Current process id in doflash: ","Current thread id in doflash: "); 
	try {
		if(mcu == "atmega32u4" || AddtoPortList(device))
			{
				#if !defined  _WIN32 || _WIN64	
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
				#if !defined  _WIN32 || _WIN64
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

				#if defined  _WIN32 || _WIN64
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
				flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
			}
		else{
				CodebenderccAPI::debugMessage("Port is in use, choose another port",3);
				flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-22));
			}	
		} 
	catch (...) 
	{
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflash exception",2);
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
		RemovePortFromList(device);
    }
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflash ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::doFlash() threw an unknown exception");
}

void CodebenderccAPI::doflashWithProgrammer(const std::string& device, const std::string& code, const std::string& maxsize, std::map<std::string, std::string>& programmerData, const std::string& mcu, const FB::JSObjectPtr & flash_callback) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer",3);
	std::string os = getPlugin().get()->getOS();
	CodebenderccAPI::getThreadId("Current process id in doflashWithProgrammer: ","Current thread id in doflashWithProgrammer: "); 
	try {
			if((programmerData["communication"] == "usb")||(programmerData["communication"] == "")||(AddtoPortList(device)))
			{
				#if !defined  _WIN32 || _WIN64	
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

				#if defined  _WIN32 || _WIN64
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
				CodebenderccAPI::debugMessage("CodebenderccAPI::Port is already in use.",3);
			}
	}catch(...){
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer exception",2);
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
		RemovePortFromList(device);
    }
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::doflashWithProgrammer() threw an unknown exception");
}

void CodebenderccAPI::doflashBootloader(const std::string& device,  std::map<std::string, std::string>& programmerData, std::map<std::string, std::string>& bootloaderData, const std::string& mcu, const FB::JSObjectPtr & flash_callback) try {

	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader",3);
	CodebenderccAPI::getThreadId("Current process id in doflashBootloader: ","Current thread id in doflashBootloader: "); 
	std::string os = getPlugin().get()->getOS();
	try {
		if((programmerData["communication"] == "usb")||(programmerData["communication"] == "")||(AddtoPortList(device)))
		{
			#if !defined  _WIN32 || _WIN64	
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
						#if defined  _WIN32 || _WIN64
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
			CodebenderccAPI::debugMessage("CodebenderccAPI::Port is already in use:",3);
		}	
	}catch(...){
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader exception",2);
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
		RemovePortFromList(device);
	}
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::doflashBootloader() threw an unknown exception");
}

const std::string CodebenderccAPI::setProgrammerCommand(std::map<std::string, std::string>& programmerData) try {
	
	CodebenderccAPI::debugMessage("CodebenderccAPI::setProgrammerCommand",3);
	std::string os = getPlugin().get()->getOS();
	#if !defined  _WIN32 || _WIN64
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
	#if defined  _WIN32 || _WIN64
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
		std::string unixCommand = command; 
		retval = unixExecAvrdude(unixCommand);
	#endif
		/** Print the content of the output file, if debugging is on. **/
	if (CodebenderccAPI::checkDebug())
	{
		std::ifstream ifs(errfile.c_str());
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

int CodebenderccAPI::unixExecAvrdude (const std::string &unixExecCommand)
{ 
	/* Split command and store exec arguments in a string vector */
	std::istringstream StreamCommand(unixExecCommand);
	std::string comArg;
	std::vector<std::string> args;
	std::vector<std::string>::const_iterator iterator;

	while(std::getline(StreamCommand, comArg, ' ')) {
		comArg.erase(remove( comArg.begin(), comArg.end(), '\"' ),comArg.end());
		args.push_back(comArg);
	}

	/* Convert string vector to char array */
	std::vector<char *> cmd_argv(args.size() + 1);
	for (std::size_t i = 0; i != args.size(); i++) {
		cmd_argv[i] = &args[i][0];
	}

    pid_t cpid, w;
    
    /* make a duplicate of the current process */
    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

	/* Code executed by child process */
    if (cpid == 0) 
    {
    	const char * frpathout = outfile.c_str();
    	const char * frpatherr = errfile.c_str();		
		stdout=freopen(frpathout,"w", stdout);
		stderr=freopen(frpatherr,"w",stderr);
       	execv(cmd_argv[0], cmd_argv.data());
    } 
     /* Code executed by parent process */
    else 
    {
        long oldSize=0;
        long newSize=0;
       	int counter =0;
        do {
        	int status = 0;
            w = waitpid(cpid, &status, WNOHANG);
            /* On error, -1 is returned. */
            if (w == -1){
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            /* If WNOHANG was specified  as argument in waitpid and child specified by pid exists, 
				but have not yet changed state, then 0 is returned. */
            if (w == 0)
            {
				const char *patherr = errfile.c_str();		
	    		oldSize=CodebenderccAPI::filesize(patherr);
			        if (newSize != oldSize) 
			        	newSize=oldSize;
		   			else{
		   				if(counter<10)
		    				counter++;
		    			else{
							kill (cpid, SIGKILL);
							continue;
							}
		   				}	
	        	sleep(1);
            }
            /* On success waitpid(), returns the process ID of the child whose state has changed */
			else{
				if(WIFSIGNALED(status)) {
	        		return WTERMSIG(status);
				}
				if (WIFEXITED(status)) {
	            	return WEXITSTATUS(status);
           		 } 

				}						
       } while (1);
    }
    exit(EXIT_SUCCESS);
}

long CodebenderccAPI::filesize(const char *filename)
{
FILE *f = fopen(filename,"rb");  /* open the file in read only */
long size = 0;
  if (fseek(f,0,SEEK_END)==0) /* seek was successful */
      size = ftell(f);
  fclose(f);
  return size;
}

/**
 * Save the binary data to the binary file specified in the constructor.
 */
void CodebenderccAPI::saveToBin(unsigned char * data, size_t size) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::saveToBin",3);
	
	std::ofstream myfile;
    myfile.open(binFile.c_str(), std::fstream::binary);
    for (size_t i = 0; i < size; i++) {

        myfile << data[i];
    }
    myfile.close();
	CodebenderccAPI::debugMessage("CodebenderccAPI::saveToBin ended",3);
} catch (...) {
    error_notify("CodebenderccAPI::saveToBin() threw an unknown exception");
}

/**
 * Save the hex data of the bootloader file to the hex file specified in the constructor.
 */
void CodebenderccAPI::saveToHex(const std::string& hexContent) try {
	CodebenderccAPI::debugMessage("CodebenderccAPI::saveToHex",3);

	// Save the content of the bootloader to a local hex file.
	std::ofstream myfile;
    myfile.open(hexFile.c_str(), std::fstream::binary);

	myfile << hexContent;
	myfile.close();

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
#if !defined  _WIN32 || _WIN64	
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    while (fgets(buffer, 128, pipe) != NULL) {

        result += buffer;
    }
    pclose(pipe);
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