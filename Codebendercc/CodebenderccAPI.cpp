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

FB::variant CodebenderccAPI::flash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr &flash_callback) {
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::flash");
	
	if (!validate_device(device)) return -1;
    if (!validate_code(code)) return -2;
    if (!validate_number(maxsize)) return -3;
    if (!validate_number(speed)) return -4;
    if (!validate_charnum(protocol)) return -5;
    if (!validate_charnum(mcu)) return -6;

    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::doflash,
            this, device, code, maxsize, protocol, speed, mcu, flash_callback));

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::flash ended");
    return 0;
}

bool CodebenderccAPI::setCallback(const FB::JSObjectPtr &callback) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::setCallback");
	
	callback_ = callback;

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::setCallback ended");
    return true;
}

void CodebenderccAPI::openPort(const std::string &port, const unsigned int &baudrate) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::openPort");
	
	std::string device;
	device = port;
	
	#if defined _WIN32||_WIN64
		device = "\\\\.\\" + port;
	#endif

	try{
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
	}catch(...){
		if (CodebenderccAPI::checkDebug())		//debugging
			m_host->htmlLog("CodebenderccAPI::openPort exception");
		perror("Error opening port.");
	}
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::openPort ended");

}

void CodebenderccAPI::closePort() {
	
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::closePort");
		
	try{
		if(serialPort.isOpen())
			serialPort.close();
	}catch(...){
		if (CodebenderccAPI::checkDebug())		//debugging
			m_host->htmlLog("CodebenderccAPI::closePort exception");
	}

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::closePort ended");
}

#if defined _WIN32||_WIN64

std::string CodebenderccAPI::QueryKey(HKEY hKey) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::QueryKey");

    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name (characters)
    DWORD    cbMaxValueData;       // longest value data (bytes)
    
    DWORD i, retCode; 
 
    TCHAR	achValue[MAX_KEY_LENGTH]; 
    DWORD	cchValue = MAX_KEY_LENGTH; 
	
	std::string ports = "";
	
	// Get the registry key value count. 
	retCode = RegQueryInfoKey(
        hKey,		             // An open registry key handle.
        NULL,
        NULL,
        NULL,
        NULL,
        NULL, 
        NULL,
        &cValues,                // A pointer to a variable that receives the number of values that are associated with the key.
        &cchMaxValue,            // A pointer to a variable that receives the size of the key's longest value name, in Unicode characters.  
        &cbMaxValueData,         // A pointer to a variable that receives the size of the longest data component among the key's values, in bytes.
        NULL, 
        NULL);

    
    // Enumerate the key values. 

	BYTE* buffer = new BYTE[cbMaxValueData];

    if (cValues)				// cValues is the count of values found in the SERIALCOMM subkey
    {
		for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
        { 
			cchValue = MAX_KEY_LENGTH; 
            achValue[0] = '\0'; 
            retCode = RegEnumValue(
				hKey,			// A handle to an open registry key
				i,				// The index of the value to be retrieved
                achValue,		// A pointer to a buffer that receives the name of the value as a null-terminated string
                &cchValue,		// A pointer to a variable that specifies the size of the buffer pointed to by the achValue parameter, in characters.
                NULL,			// lpReserved parameter is reserved and must be NULL
                NULL,			
                NULL,			
                NULL);			

            if (retCode == ERROR_SUCCESS ) 
            { 
				DWORD lpData = cbMaxValueData;
				buffer[0] = '\0';
				
				LONG dwRes = RegQueryValueEx(hKey, achValue, 0, NULL, (LPBYTE)buffer, &lpData);
				
				if (dwRes == ERROR_SUCCESS)
				{
					std::string str( reinterpret_cast<char const*>(buffer) , (int) lpData ) ;
					std::string tmp="";
					for (int k=0;k<(int)lpData;k++)
                        {
							if(buffer[k]!=0)		// Check the buffer for bytes that contain zeros.
								tmp+=buffer[k];
	                    }
					
					ports.append(tmp);		// Append port to the list of ports.
					if (i != (cValues - 1))
						ports.append(",");
						
				}
			} 
        }
		
    }
	delete [] buffer;
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::QueryKey ended");
	return ports;
}

std::string CodebenderccAPI::probeUSB() {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::probeUSB");

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
   
	RegCloseKey(hKey);	// Need to close the key handle after the task is completed.

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::probeUSB ended");

    return ports;
}
#else

std::string CodebenderccAPI::probeUSB() {
	
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::probeUSB");

    DIR *dp;
    std::string dirs = "";
	struct dirent *ep;
    dp = opendir("/dev/");
    if (dp != NULL) {
        while (ep = readdir(dp)) {
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
		if (CodebenderccAPI::checkDebug())		//debugging
			m_host->htmlLog("CodebenderccAPI::probeUSB could not open directory");
        perror("Couldn't open the directory");
	}
	
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::probeUSB ended");

    return dirs;
}
#endif

void CodebenderccAPI::enableDebug() {
	if(CodebenderccAPI::checkDebug() == false)
		debug_ = true;
}

void CodebenderccAPI::disableDebug() {
	if(CodebenderccAPI::checkDebug() == true)
		debug_ = false;
}

bool CodebenderccAPI::checkDebug() {
	return debug_;
}


CodebenderccPtr CodebenderccAPI::getPlugin() {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::getPlugin");

	CodebenderccPtr plugin(m_plugin.lock());
    if (!plugin) {
		if (CodebenderccAPI::checkDebug())		//debugging
			m_host->htmlLog("CodebenderccAPI::getPlugin invalid plugin");
        throw FB::script_error("The plugin is invalid");
    }
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::getPlugin ended");

    return plugin;
}

// Read-only property version

std::string CodebenderccAPI::get_version() {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::get_version");

	return FBSTRING_PLUGIN_VERSION;
}

FB::variant CodebenderccAPI::getLastCommand() {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::getLastCommand");

	return lastcommand;
}

FB::variant CodebenderccAPI::getFlashResult() {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::getFlashResult");

	FILE *pFile;
	#if defined _WIN32 || _WIN64
		std::string filename = FB::wstring_to_utf8(outfile);
		pFile = fopen(filename.c_str(), "r");
	#else
		pFile = fopen(outfile.c_str(), "r");
	#endif
    char buffer[128];
    std::string result = "";
    while (!feof(pFile)) {
        if (fgets(buffer, 128, pFile) != NULL)
            result += buffer;
    }
    fclose(pFile);

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::getFlashResult ended");
    return result;
}

void CodebenderccAPI::serialWrite(const std::string & message) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::serialWrite");

    std::string mess = message;
	size_t bytes_read;
	try
	{
		if(serialPort.isOpen()){
			
			bytes_read = serialPort.write(mess);
			if(bytes_read != 0){
				perror("Wrote to port");
				if (CodebenderccAPI::checkDebug())		//debugging
					m_host->htmlLog("Wrote to port: " + mess);
			}
		}
		else {
			if (CodebenderccAPI::checkDebug())		//debugging
				m_host->htmlLog("CodebenderccAPI::serialWrite port not open");
			perror("null");
		}
	}catch(...){
		if (CodebenderccAPI::checkDebug())		//debugging
			m_host->htmlLog("CodebenderccAPI::serialWrite open serial port exception");
		notify("disconnect");
	}
	
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::serialWrite ended");

}

FB::variant CodebenderccAPI::disconnect() {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::disconnect");

	if(!(serialPort.isOpen()))
		return 1;
	try{
			CodebenderccAPI::closePort();
		}catch(...){
			if (CodebenderccAPI::checkDebug())		//debugging
				m_host->htmlLog("CodebenderccAPI::disconnect close port exception");
		}

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::disconnect ended");

	return 1;
	
}


#if defined _WIN32 || _WIN64

int CodebenderccAPI::execAvrdude(const std::wstring & command) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::execAvrdude");

	DWORD dwExitCode = -1;
   
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
		GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		&sa,
		OPEN_ALWAYS, 
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
		perror("Failed to create child process.");
		return -8;
	}
 
	// Wait until child processes exit. Don't wait forever.
	WaitForSingleObject( pi.hProcess, INFINITE );
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	TerminateProcess( pi.hProcess, 0 ); // Kill process if it is still running

	// CreateProcess docs specify that these must be closed. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );	

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::execAvrdude ended");
	
	return dwExitCode;

}
#endif

bool CodebenderccAPI::serialRead(const std::string &port, const std::string &baudrate, const FB::JSObjectPtr &callback) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::serialRead");

	std::string message = "connecting at ";
    message += baudrate;

    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(message));

    unsigned int brate = boost::lexical_cast<unsigned int, std::string > (baudrate);

    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::serialReader, this, port, brate, callback));
    if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::serialRead ended");
	return true; // the thread is started
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////PRIVATE////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CodebenderccAPI::doflash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr & flash_callback) {
    
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::doflash");

	std::string os = getPlugin().get()->getOS();

	try {
#if !defined  _WIN32 || _WIN64	
        chmod(avrdude.c_str(), S_IRWXU);
#else		
		std::ofstream batchFd; // if on Windows, create the file descriptor for the batch file.
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
				CodebenderccAPI::openPort(fdevice, 1200);
				delay(2000);
				CodebenderccAPI::closePort();
				// delay for 300 ms so that the reset is complete
				if ((os == "Windows") || (os == "X11"))
					delay(300);
			}catch (...) {
				if (CodebenderccAPI::checkDebug())		//debugging
					m_host->htmlLog("CodebenderccAPI::doflash exception thrown while opening serial port");
				perror("Error opening leonardo port");
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
			if (CodebenderccAPI::checkDebug()){
				m_host->htmlLog("Listing serial port changes..");
				m_host->htmlLog("Ports : {" + oldports + "}");
			}
            
			// Get the list of ports until the port 
			int elapsed_time = 0;
			bool found = false;
            while(elapsed_time <= 10000){
                std::vector<std::string> newPorts;
                std::string newports = probeUSB();
                perror(newports.c_str());
                std::stringstream ss(newports);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    newPorts.push_back(item);
                }

				if (CodebenderccAPI::checkDebug())
					m_host->htmlLog("Ports : {" + newports + "}");

				// Check if the new list of ports contains a port that did not exist in the previous list.
                for (std::vector<std::string>::iterator it = newPorts.begin(); it != newPorts.end(); ++it) {
					if (std::find(oldPorts.begin(), oldPorts.end(), *it) == oldPorts.end()){
						fdevice = *it;
						found = true;
						break;
					}
                }

				if (found){		// The new leonardo port appeared in the list. Save it and go on..
					if (CodebenderccAPI::checkDebug())
						m_host->htmlLog("Found leonardo device on " + fdevice + " port");
					break;
				}

				oldPorts = newPorts;
				delay(250);
				elapsed_time += 250;

				// If a certain ammount of time has gone by, and the initial port is in the lost of ports, upload using this port
				if(((os == "Widnows" && elapsed_time >= 500) || elapsed_time >=5000) && (std::find(oldPorts.begin(), oldPorts.end(), fdevice) != oldPorts.end()) ){
					if (CodebenderccAPI::checkDebug())
						m_host->htmlLog("Uploading using selected port: " + fdevice);
					break;
				}

                if (elapsed_time == 10000) {
                    notify("Could not auto-reset or detect a manual reset!");
                    flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-1));
                    return;
                }
            }
        }

        int retVal = 1;

#if !defined  _WIN32 || _WIN64	 
		std::string command = "\"" + avrdude + "\""
                + " -C\"" + avrdudeConf + "\""
				+ " -P" + fdevice
                + " -p" + mcu
                + " -u -D -U flash:w:\"" + binFile + "\":a"
                + " -c" + protocol
                + " -b" + speed
                + " -F 2> " + "\"" + outfile + "\"";

		lastcommand = command;

        retVal = system(command.c_str());
#else

		/** 
		  *In order to avoid messing with non-ascii characters in Windows paths, prepare the command and save it 
		  * to a batch file. The command components need not specify the full path of the file, only the file name.
		  **/
		std::string batchFileContent = avrdude 
			+ " -C" + avrdudeConf
			+ " -P\\\\.\\"+ fdevice
			+ " -p" + mcu
			+ " -u -D -U flash:w:file.bin:a -c" 
			+ protocol
			+ " -b" + speed
			+ " -F";
		
		try{
			batchFd.open(batchFile.c_str());
			batchFd << batchFileContent;
			batchFd.close();
		}catch(...){
			if (CodebenderccAPI::checkDebug())
				m_host->htmlLog("Failed to write command to batch file!");
		}	

		lastcommand = batchFileContent;
		
		retVal = execAvrdude(batchFile);

#endif

        _retVal = retVal;
		perror(lastcommand.c_str());
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
		flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
    } catch (...) {
		if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::doflash exception");
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
    }
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::doflash ended");
}

/**
 * Save the binary data to the binary file specified in the constructor.
 */
void CodebenderccAPI::saveToBin(unsigned char * data, size_t size) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::saveToBin");
	
	std::ofstream myfile;
    myfile.open(binFile.c_str(), std::fstream::binary);
    for (size_t i = 0; i < size; i++) {

        myfile << data[i];
    }
    myfile.close();

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::saveToBin ended");
}

/**
 * decode base64 encoded data
 *
 * @param source the encoded data (zero terminated)
 * @param target pointer to the target buffer
 * @param targetlen length of the target buffer
 * @return length of converted data on success, -1 otherwise
 */
size_t CodebenderccAPI::base64_decode(const char *source, unsigned char *target, size_t targetlen) {
    
	char *src, *tmpptr;
    char quadruple[4];
    unsigned char tmpresult[3];
    int i;
    size_t tmplen = 3;
    int converted = 0;

    /* concatinate '===' to the source to handle unpadded base64 data */
    src = (char *) malloc(strlen(source) + 5);
    if (src == NULL)
        return -1;
    strcpy(src, source);
    strcat(src, "====");
    tmpptr = src;

    /* convert as long as we get a full result */
    while (tmplen == 3) {
        /* get 4 characters to convert */
        for (i = 0; i < 4; i++) {
            /* skip invalid characters - we won't reach the end */
            while (*tmpptr != '=' && _base64_char_value(*tmpptr) < 0)
                tmpptr++;

            quadruple[i] = *(tmpptr++);
        }

        /* convert the characters */
        tmplen = _base64_decode_triple(quadruple, tmpresult);

        /* check if the fit in the result buffer */
        if (targetlen < tmplen) {
            free(src);
            return -1;
        }

        /* put the partial result in the result buffer */
        memcpy(target, tmpresult, tmplen);
        target += tmplen;
        targetlen -= tmplen;
        converted += tmplen;
    }

    free(src);

    return converted;
}

/**
 * determine the value of a base64 encoding character
 *
 * @param base64char the character of which the value is searched
 * @return the value in case of success (0-63), -1 on failure
 */
int CodebenderccAPI::_base64_char_value(char base64char) {

	if (base64char >= 'A' && base64char <= 'Z')
        return base64char - 'A';
    if (base64char >= 'a' && base64char <= 'z')
        return base64char - 'a' + 26;
    if (base64char >= '0' && base64char <= '9')
        return base64char - '0' + 2 * 26;
    if (base64char == '+')
        return 2 * 26 + 10;
    if (base64char == '/')

        return 2 * 26 + 11;
    return -1;
}

/**
 * decode a 4 char base64 encoded byte triple
 *
 * @param quadruple the 4 characters that should be decoded
 * @param result the decoded data
 * @return lenth of the result (1, 2 or 3), 0 on failure
 */
int CodebenderccAPI::_base64_decode_triple(char quadruple[4], unsigned char *result) {

	int i, triple_value, bytes_to_decode = 3, only_equals_yet = 1;
    int char_value[4];

    for (i = 0; i < 4; i++)
        char_value[i] = _base64_char_value(quadruple[i]);

    /* check if the characters are valid */
    for (i = 3; i >= 0; i--) {
        if (char_value[i] < 0) {
            if (only_equals_yet && quadruple[i] == '=') {
                /* we will ignore this character anyway, make it something
                 * that does not break our calculations */
                char_value[i] = 0;
                bytes_to_decode--;
                continue;
            }
            return 0;
        }
        /* after we got a real character, no other '=' are allowed anymore */
        only_equals_yet = 0;
    }

    /* if we got "====" as input, bytes_to_decode is -1 */
    if (bytes_to_decode < 0)
        bytes_to_decode = 0;

    /* make one big value out of the partial values */
    triple_value = char_value[0];
    triple_value *= 64;
    triple_value += char_value[1];
    triple_value *= 64;
    triple_value += char_value[2];
    triple_value *= 64;
    triple_value += char_value[3];

    /* break the big value into bytes */
    for (i = bytes_to_decode; i < 3; i++)
        triple_value /= 256;
    for (i = bytes_to_decode - 1; i >= 0; i--) {

        result[i] = triple_value % 256;
        triple_value /= 256;
    }

    return bytes_to_decode;
}

void CodebenderccAPI::serialReader(const std::string &port, const unsigned int &baudrate, const FB::JSObjectPtr & callback) {
	
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::serialReader");
		
	try {
		CodebenderccAPI::openPort(port, baudrate);

		int d;
		std::string rcvd;
		
		for (;;) {
			if(serialPort.isOpen())
				rcvd = "";	

				rcvd = serialPort.read((size_t) 1);
				if(rcvd != ""){
					d = (int) rcvd[0];
					if (CodebenderccAPI::checkDebug())		//debugging
						m_host->htmlLog("Received character: " + rcvd[0]);
					callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(d));
				}
		}
    } catch (...) {
		if (CodebenderccAPI::checkDebug())		//debugging
			m_host->htmlLog("CodebenderccAPI::serialReader loop interrupted");
        notify("disconnect");
    }
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::serialReader ended");
}

std::string CodebenderccAPI::exec(const char * cmd) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::exec");

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
	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::exec ended");
    return result;
}

void CodebenderccAPI::notify(const std::string &message) {

	if (CodebenderccAPI::checkDebug())		//debugging
		m_host->htmlLog("CodebenderccAPI::notify");

	callback_->InvokeAsync("", FB::variant_list_of(shared_from_this())(message.c_str()));
}


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////validations//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool CodebenderccAPI::validate_number(const std::string & input) {

	try {
        boost::lexical_cast<double>(input);
        return true;
    } catch (boost::bad_lexical_cast &) {

        return false;
    }
}

bool CodebenderccAPI::validate_device(const std::string & input) {
	
	static const boost::regex acm("\\/dev\\/ttyACM[[:digit:]]+");
    static const boost::regex usb("\\/dev\\/ttyUSB[[:digit:]]+");
    static const boost::regex com("COM[[:digit:]]+");
    static const boost::regex cu("\\/dev\\/cu.[0-9a-zA-Z\\-]+");

    return boost::regex_match(input, acm)
            || boost::regex_match(input, usb)
            || boost::regex_match(input, com)
            || boost::regex_match(input, cu)
            ;
}

bool CodebenderccAPI::validate_code(const std::string & input) {

	static const boost::regex base64("[0-9a-zA-Z+\\/=\n]+");

    return boost::regex_match(input, base64);
}

bool CodebenderccAPI::validate_charnum(const std::string & input) {

	static const boost::regex charnum("[0-9a-zA-Z]*");
    return boost::regex_match(input, charnum);
}
