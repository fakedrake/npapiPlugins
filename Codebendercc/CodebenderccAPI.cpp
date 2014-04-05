/**
 * Auto-generated CodebenderccAPI.cpp
*/
#include "CodebenderccAPI.h"

/**
 * deprecated
*/
FB::variant
CodebenderccAPI::download()
{
    return "deprecated";
}

/**
 * public
*/
FB::variant
CodebenderccAPI::flash(const std::string &device,
                       const std::string &code,
                       const std::string &maxsize,
                       const std::string &protocol,
                       const std::string &speed,
                       const std::string &mcu,
                       const FB::JSObjectPtr &flash_callback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::flash", 3);

#if defined _WIN32 || _WIN64
    /* Check if finding the short path of the plugin failed. */
    if (current_dir == L"") {
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-2));
        return 0;
    }
#endif

    int error_code = 0;

    if (!validate_device(device))
        error_code = -4;
    if (!validate_code(code))
        error_code = -5;
    if (!validate_number(maxsize))
        error_code = -6;
    if (!validate_number(speed))
        error_code = -7;
    if (!validate_charnum(protocol))
        error_code = -8;
    if (!validate_charnum(mcu))
        error_code = -9;

    if (error_code != 0) {
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(error_code));
        return 0;
    }

    boost::thread* t =
        new boost::thread(boost::bind(&CodebenderccAPI::doflash,
                                      this,
                                      device,
                                      code,
                                      maxsize,
                                      protocol,
                                      speed,
                                      mcu,
                                      flash_callback));

    CodebenderccAPI::debugMessage("CodebenderccAPI::flash ended", 3);

    return 0;
}

FB::variant
CodebenderccAPI::flashWithProgrammer(const std::string &device,
                                     const std::string &code,
                                     const std::string &maxsize,
                                     const std::string &programmerProtocol,
                                     const std::string &programmerCommunication,
                                     const std::string &programmerSpeed,
                                     const std::string &programmerForce,
                                     const std::string &programmerDelay,
                                     const std::string &mcu,
                                     const FB::JSObjectPtr &cback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::flashWithProgrammer", 3);

#if defined _WIN32 || _WIN64
    /* Check if finding the short path of the plugin failed. */
    if (current_dir == L"") {
        cback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-2));
        return 0;
    }
#endif

    /**
     * Input validation. The error codes returned correspond to
     * messages printed by the javascript of the website
    */
    if (!validate_code(code))
        return -2;
    if (!validate_number(maxsize))
        return -3;

    std::map<std::string, std::string> programmerData;

    int progValidation = programmerPrefs(device,
                                         programmerProtocol,
                                         programmerSpeed,
                                         programmerCommunication,
                                         programmerForce,
                                         programmerDelay,
                                         mcu,
                                         programmerData);
    if (progValidation != 0) {
        cback->InvokeAsync("", FB::variant_list_of(shared_from_this())(progValidation));
        return 0;
    }

    boost::thread* t =
        new boost::thread(boost::bind(&CodebenderccAPI::doflashWithProgrammer,
                                      this,
                                      device,
                                      code,
                                      maxsize,
                                      programmerData,
                                      mcu,
                                      cback));

    CodebenderccAPI::debugMessage("CodebenderccAPI::flashWithProgrammer ended", 3);

    return 0;
}

FB::variant
CodebenderccAPI::flashBootloader(const std::string &device,
                                 const std::string &programmerProtocol,
                                 const std::string &programmerCommunication,
                                 const std::string &programmerSpeed,
                                 const std::string &programmerForce,
                                 const std::string &programmerDelay,
                                 const std::string &highFuses,
                                 const std::string &lowFuses,
                                 const std::string &extendedFuses,
                                 const std::string &unlockBits,
                                 const std::string &lockBits,
                                 const std::string &mcu,
                                 const FB::JSObjectPtr &cback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::flashBootloader", 3);

#if defined _WIN32 || _WIN64
    /* Check if finding the short path of the plugin failed. */
    if (current_dir == L"") {
        cback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-2));
        return 0;
    }
#endif

    /**
     * Input validation. The error codes returned correspond to
     * messages printed by the javascript of the website
    */
    std::map<std::string, std::string> programmerData;

    int progValidation = programmerPrefs(device,
                                         programmerProtocol,
                                         programmerSpeed,
                                         programmerCommunication,
                                         programmerForce,
                                         programmerDelay,
                                         mcu,
                                         programmerData);
    if (progValidation != 0) {
        cback->InvokeAsync("", FB::variant_list_of(shared_from_this())(progValidation));
        return 0;
    }

    std::map<std::string, std::string> bootloaderData;

    int bootValidation = bootloaderPrefs(lowFuses,
                                         highFuses,
                                         extendedFuses,
                                         unlockBits,
                                         lockBits,
                                         bootloaderData);
    if (bootValidation != 0) {
        cback->InvokeAsync("", FB::variant_list_of(shared_from_this())(bootValidation));
        return 0;
    }

    boost::thread* t =
        new boost::thread(boost::bind(&CodebenderccAPI::doflashBootloader,
                                      this,
                                      device,
                                      programmerData,
                                      bootloaderData,
                                      mcu,
                                      cback));

    CodebenderccAPI::debugMessage("CodebenderccAPI::flashBootloader ended", 3);

    return 0;
}

bool
CodebenderccAPI::setCallback(const FB::JSObjectPtr &callback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::setCallback", 3);

    callback_ = callback;

    CodebenderccAPI::debugMessage("CodebenderccAPI::setCallback ended", 3);

    return true;
}

void
CodebenderccAPI::openPort(const std::string &port,
                          const unsigned int &baudrate)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::openPort", 3);

    std::string device;
    device = port;

#if defined _WIN32 || _WIN64
    device = "\\\\.\\" + port;
#endif

    try
    {
        if (serialPort.isOpen() == false)
        {
        #if defined _WIN32 || _WIN64
            /* need to set a zero timeout when on Windows */
            portTimeout = Timeout(std::numeric_limits<uint32_t>::max(),
                                  0, 0, 0, 0);
        #else
            portTimeout = Timeout(std::numeric_limits<uint32_t>::max(),
                                  1000, 0, 1000, 0);
        #endif
            /* set port name */
            serialPort.setPort(device);
            /* set port baudrate */
            serialPort.setBaudrate(baudrate);
            /* set the read/write timeout of the port */
            serialPort.setTimeout(portTimeout);

			/* open the port */
            serialPort.open();
            /* set Data Transfer signal, needed for Arduino Leonardo */
            serialPort.setDTR(true);
            /* set Request to Send signal to false, needed for Arduino
               Leonardo */
            serialPort.setRTS(false);
        }
    }
    catch(...)
    {
        CodebenderccAPI::debugMessage("CodebenderccAPI::openPort exception", 2);
        perror("Error opening port.");
    }

    CodebenderccAPI::debugMessage("CodebenderccAPI::openPort ended", 3);
}

void
CodebenderccAPI::closePort()
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::closePort", 3);

    try
    {
        if(serialPort.isOpen())
            serialPort.close();
    }
    catch(...)
    {
        CodebenderccAPI::debugMessage("CodebenderccAPI::closePort exception", 2);
    }

    CodebenderccAPI::debugMessage("CodebenderccAPI::closePort ended", 3);
}

#if defined _WIN32 || _WIN64
std::string
CodebenderccAPI::QueryKey(HKEY hKey)
{
    /* number of values for key */
    DWORD cValues;
    /* longest value name (characters) */
    DWORD cchMaxValue;
    /* longest value data (bytes) */
    DWORD cbMaxValueData;

    TCHAR	achValue[MAX_KEY_LENGTH];
    DWORD	cchValue = MAX_KEY_LENGTH;

    DWORD i, retCode;
    std::string ports = "";

    /* Get the registry key value count. */
    retCode = CodebenderccAPI::RegQueryInfoKey(
            /* An open registry key handle. */
            hKey,
            NULL, NULL, NULL, NULL, NULL, NULL,
            /* A pointer to a variable that receives the number of values
               that are associated with the key. */
            &cValues,
            /* A pointer to a variable that receives the size of the key's
               longest value name, in Unicode characters.  */
            &cchMaxValue,
            /* A pointer to a variable that receives the size of the longest
               data component among the key's values, in bytes. */
            &cbMaxValueData,
            NULL, NULL);


    /*
     * Enumerate the key values.
    */

    BYTE* buffer = new BYTE[cbMaxValueData];

    /* cValues is the count of values found in the SERIALCOMM subkey */
    if (cValues)
    {
        for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            cchValue = MAX_KEY_LENGTH;
            achValue[0] = '\0';
            retCode = CodebenderccAPI::RegEnumValue(
                    /* A handle to an open registry key */
                    hKey,
                    /* The index of the value to be retrieved */
                    i,
                    /* A pointer to a buffer that receives the name of
                       the value as a null-terminated string */
                    achValue,
                    /* A pointer to a variable that specifies the size of the
                       buffer pointed to by the achValue parameter, in
                       characters. */
                    &cchValue,
                    /* lpReserved parameter is reserved and must be NULL */
                    NULL,
                    NULL, NULL, NULL);

            if (retCode == ERROR_SUCCESS)
            {
                DWORD lpData = cbMaxValueData;
                buffer[0] = '\0';

                LONG dwRes = CodebenderccAPI::RegQueryValueEx(hKey,
                                                              achValue,
                                                              0,
                                                              NULL,
                                                              (LPBYTE) buffer,
                                                              &lpData);
                if (dwRes == ERROR_SUCCESS)
                {
                    std::string str(reinterpret_cast<char const*>(buffer), (int) lpData);
                    std::string tmp="";
                    for (int k = 0; k < (int) lpData; k++)
                    {
                        /* Check the buffer for bytes that contain zeros. */
                        if(buffer[k] != 0)
                            tmp+=buffer[k];
                    }

                    /* Append port to the list of ports. */
                    ports.append(tmp);
                    if (i != (cValues - 1))
                        ports.append(",");
                }
            }
        }
    }

    delete [] buffer;
    return ports;
}

std::string
CodebenderccAPI::probeUSB()
{
    CodebenderccAPI::debugMessageProbe("CodebenderccAPI::probeUSB probing USB ports", 3);

    std::string ports ="";
    HKEY hKey;

    /*
     * Open the registry key where serial port key-value pairs are stored.
    */
    if (CodebenderccAPI::RegOpenKeyEx(
                /* The name of the registry key handle is always the same. */
                HKEY_LOCAL_MACHINE,
                /* The same applies to the subkey, since we are looking for
                   serial ports only. */
                TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM\\"),
                0,
                /* Set the access rights, before reading the key contents. */
                KEY_READ,
                /* Set the variable that revieves the open key handle. */
                &hKey) == ERROR_SUCCESS)
    {
        /* Call QueryKey function to retrieve the available ports. */
        ports.append(CodebenderccAPI::QueryKey(hKey));
    }

    if (lastPortCount != ports.length()) {
        lastPortCount = ports.length();
        CodebenderccAPI::detectNewPort(ports);
    }

    /* Need to close the key handle after the task is completed. */
    CodebenderccAPI::RegCloseKey(hKey);
    return ports;
}

#else

std::string
CodebenderccAPI::probeUSB()
{
    CodebenderccAPI::debugMessageProbe("CodebenderccAPI::probeUSB probing USB ports", 3);

    DIR *dp;
    std::string dirs = "";
    struct dirent *ep;

    dp = CodebenderccAPI::opendir("/dev/");
    if (dp == NULL)
        return dirs;

    while (ep = CodebenderccAPI::readdir(dp))
    {
        //UNIX ARDUINO PORTS
        if (boost::contains(ep->d_name, "ttyACM") ||
            boost::contains(ep->d_name, "ttyUSB") ||
            boost::contains(ep->d_name, "cu."))
        {
            dirs += "/dev/";
            dirs += ep->d_name;
            dirs += ",";
        }
    }

    CodebenderccAPI::closedir(dp);

    if (lastPortCount != dirs.length()) {
        lastPortCount = dirs.length();
        CodebenderccAPI::detectNewPort(dirs);
    }

    return dirs;
}
#endif

void
CodebenderccAPI::enableDebug(int debugLevel)
{
    if (debugLevel >= 1 && debugLevel <= 3)
    {
        debug_ = true;
        currentLevel = debugLevel;
        if (currentLevel == 3)
        {
            if (!(debugFile.is_open()))
            {
                debugFile.open(debugFilename.c_str());
            }
        }
        else
        {
            if (debugFile.is_open())
            {
                debugFile.close();
            }
        }
    }
    else
    {
        if (debugFile.is_open())
        {
            debugFile.close();
        }

        m_host->htmlLog("Level set in enableDebug is not valid.");
    }
}

void
CodebenderccAPI::disableDebug()
{
    debug_ = false;

    if (currentLevel == 3)
    {
        if (debugFile.is_open())
        {
            debugFile.close();
        }
    }
}

bool
CodebenderccAPI::checkDebug()
{
	return debug_;
}

void
CodebenderccAPI::debugMessageProbe(const char *messageDebug,
                                   int minimumLevel)
{
    if (CodebenderccAPI::checkDebug() &&
        minimumLevel <= currentLevel &&
        probeFlag == false)
    {
        probeFlag = true;
        m_host->htmlLog(messageDebug);

        if (currentLevel == 3)
        {
            if (debugFile.is_open()) {
                debugFile << messageDebug;
                debugFile << "\n";
            }
        }
    }
}

void
CodebenderccAPI::debugMessage(const char *messageDebug,
                              int minimumLevel)
{
    if (CodebenderccAPI::checkDebug() && minimumLevel <= currentLevel)
        m_host->htmlLog(messageDebug);

    probeFlag = false;
    if (currentLevel == 3) {
        if (debugFile.is_open()) {
            debugFile << messageDebug;
            debugFile << "\n";
        }
    }
}

CodebenderccPtr
CodebenderccAPI::getPlugin()
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::getPlugin", 3);

    CodebenderccPtr plugin(m_plugin.lock());
    if (!plugin) {
        CodebenderccAPI::debugMessage("CodebenderccAPI::getPlugin invalid plugin", 3);
        throw FB::script_error("The plugin is invalid");
    }

    CodebenderccAPI::debugMessage("CodebenderccAPI::getPlugin ended", 3);

    return plugin;
}

std::string
CodebenderccAPI::get_version()
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::get_version", 3);

    return FBSTRING_PLUGIN_VERSION;
}

FB::variant
CodebenderccAPI::getLastCommand()
{
	CodebenderccAPI::debugMessage("CodebenderccAPI::getLastCommand", 3);

	return lastcommand;
}

FB::variant
CodebenderccAPI::getFlashResult()
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::getFlashResult", 3);

    FILE *pFile;
    char buffer[128];
    std::string result = "";

#if defined _WIN32 || _WIN64
    std::string filename = FB::wstring_to_utf8(outfile);
    pFile = CodebenderccAPI::fopen(filename.c_str(), "r");
#else
    pFile = CodebenderccAPI::fopen(outfile.c_str(), "r");
#endif
    if (pFile == NULL)
        return result;

    while (!feof(pFile)) {
        if (CodebenderccAPI::fgets(buffer, 128, pFile) != NULL)
            result += buffer;
    }

    CodebenderccAPI::fclose(pFile);

    CodebenderccAPI::debugMessage("CodebenderccAPI::getFlashResult ended",3);

    return result;
}

void
CodebenderccAPI::serialWrite(const std::string &message)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::serialWrite",3);

    std::string mess = message;
    size_t bytes_read;

    try
    {
        if (serialPort.isOpen())
        {
            bytes_read = serialPort.write(mess);

            if (bytes_read != 0)
            {
                perror("Wrote to port");
                std::string portMessage = "Wrote to port: " + mess + " ";
                CodebenderccAPI::debugMessage(portMessage.c_str(), 1);
            }
        }
        else
        {
            CodebenderccAPI::debugMessage("CodebenderccAPI::serialWrite port not open", 1);
            perror("null");
        }
    }
    catch (...)
    {
        CodebenderccAPI::debugMessage("CodebenderccAPI::serialWrite open serial port exception", 1);
        notify("disconnect");
    }
    CodebenderccAPI::debugMessage("CodebenderccAPI::serialWrite ended", 3);
}

FB::variant
CodebenderccAPI::disconnect()
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::disconnect",3);

    if (!serialPort.isOpen())
        return 1;

    try {
        CodebenderccAPI::closePort();
    } catch(...) {
        CodebenderccAPI::debugMessage("CodebenderccAPI::disconnect close port exception", 2);
    }

    CodebenderccAPI::debugMessage("CodebenderccAPI::disconnect ended", 3);

    return 1;
}

#if defined _WIN32 || _WIN64
int
CodebenderccAPI::winExecAvrdude(const std::wstring &command,
                                bool appendFlag)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::winExecAvrdude", 3);

    DWORD dwExitCode = -1;
    DWORD APPEND;
    DWORD CREATE;

    if (appendFlag) {
        APPEND = FILE_APPEND_DATA;
        CREATE = OPEN_ALWAYS;
    }
    else {
        APPEND = GENERIC_WRITE;
        CREATE = CREATE_ALWAYS;
    }

    /* Contains the result of the child process created below. */
    std::string strResult;

    BOOL success;

    /* Create security attributes to create pipe. */
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES) };
    /* Set the bInheritHandle flag so pipe handles are inherited by
       child process. Required.  */
    sa.bInheritHandle = TRUE;
    /* Specify a security descriptor. Required. */
    sa.lpSecurityDescriptor = NULL;

    /* Specify the necessary parameters for child process. */
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    /* STARTF_USESTDHANDLES is required. */
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    /* Prevent cmd window from flashing. Requires STARTF_USESHOWWINDOW
       in dwFlags. */
    si.wShowWindow = SW_HIDE;

    /* Create an empty process information struct. Needed to get the return
       value of the command. */
    PROCESS_INFORMATION pi = { 0 };

    HANDLE fh = CodebenderccAPI::CreateFile(
            /* Create a file handle pointing to the output file,
               in order to capture the output. */
            &outfile[0],
            APPEND,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            &sa,
            CREATE,
            FILE_FLAG_SEQUENTIAL_SCAN,
            0);

    /* Bind the stdinput, stdoutput and stderror to the output file in
       order to capture all the output of the command. */
    si.hStdOutput = fh;
    si.hStdError = fh;
    si.hStdInput = fh;

    /* Create the child process. The command simply executes the contents
       of the batch file, which is the actual command. */
    success = CodebenderccAPI::CreateProcess(
                NULL,
                /* command line */
                (LPWSTR)command.c_str(),
                /* process security attributes */
                NULL,
                /* primary thread security attributes */
                NULL,
                /* Inherit pipe handles from parent process */
                TRUE,
                /* creation flags */
                CREATE_NEW_CONSOLE,
                /* use parent's environment */
                NULL,
                /* use the plugin's directory */
                current_dir,
                /* __in, STARTUPINFO pointer */
                &si,
                /* __out, receives PROCESS_INFORMATION */
                &pi);
    if (!success)
    {
        CodebenderccAPI::debugMessage("Failed to create child process.", 1);
        return -3;
    }

    /* Wait until child processes exit. Don't wait forever. */
    CodebenderccAPI::WaitForSingleObject(pi.hProcess, INFINITE);
    CodebenderccAPI::GetExitCodeProcess(pi.hProcess, &dwExitCode);
    /* Kill process if it is still running */
    CodebenderccAPI::TerminateProcess(pi.hProcess, 0);

    CodebenderccAPI::CloseHandle(fh);
    /* CreateProcess docs specify that these must be closed. */
    CodebenderccAPI::CloseHandle(pi.hProcess);
    CodebenderccAPI::CloseHandle(pi.hThread);

    CodebenderccAPI::debugMessage("CodebenderccAPI::winExecAvrdude ended", 3);

    return dwExitCode;
}
#endif

bool
CodebenderccAPI::serialRead(const std::string &port,
                            const std::string &baudrate,
                            const FB::JSObjectPtr &callback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::serialRead", 3);

    std::string message = "connecting at ";
    message += baudrate;

    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(message));

    unsigned int brate = boost::lexical_cast<unsigned int, std::string>(baudrate);

    boost::thread* t =
        new boost::thread(boost::bind(&CodebenderccAPI::serialReader,
                                      this,
                                      port,
                                      brate,
                                      callback));

    CodebenderccAPI::debugMessage("CodebenderccAPI::serialRead ended", 3);

    return true;
}

/*
 * private
*/
void
CodebenderccAPI::doflash(const std::string &device,
                         const std::string &code,
                         const std::string &maxsize,
                         const std::string &protocol,
                         const std::string &speed,
                         const std::string &mcu,
                         const FB::JSObjectPtr &flash_callback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::doflash", 3);

	std::string os = getPlugin().get()->getOS();

	try
    {
#if !defined  _WIN32 || _WIN64
        chmod(avrdude.c_str(), S_IRWXU);
#endif
		if (mcu == "atmega32u4")
            notify(MSG_LEONARD_AUTORESET);

        unsigned char buffer [150000];
        size_t size = base64_decode(code.c_str(), buffer, 150000);
        saveToBin(buffer, size);

        std::string fdevice = device;

        if (mcu == "atmega32u4")
        {
            try
            {
                /* set the "magic" baudrate to force leonardo reset */
                CodebenderccAPI::openPort(fdevice, 1200);
                delay(2000);
                CodebenderccAPI::closePort();

                /* delay for 300 ms so that the reset is complete */
                if ((os == "Windows") || (os == "X11"))
                    delay(300);
            }
            catch (...)
            {
                CodebenderccAPI::debugMessage("CodebenderccAPI::doflash exception thrown while opening serial port", 2);
                perror("Error opening leonardo port");
            }

            /* get the list of ports before resetting the leonardo board */
            std::string oldports = probeUSB();
            perror(oldports.c_str());
            std::istringstream oldStream(oldports);
            std::string token;
            std::vector<std::string> oldPorts;

            while (std::getline(oldStream, token, ','))
                oldPorts.push_back(token);

            CodebenderccAPI::debugMessage("Listing serial port changes..", 2);
            std::string oldPortsMessage = "Ports : {" + oldports + "}";
            CodebenderccAPI::debugMessage(oldPortsMessage.c_str(), 2);

            /* Get the list of ports until the port */
            int elapsed_time = 0;
            bool found = false;

            while(elapsed_time <= 10000)
            {
                std::vector<std::string> newPorts;
                std::string newports = probeUSB();
                perror(newports.c_str());

                std::stringstream ss(newports);
                std::string item;
                while (std::getline(ss, item, ','))
                    newPorts.push_back(item);

                std::string newPortsMessage = "Ports : {" + newports + "}";
                CodebenderccAPI::debugMessage(newPortsMessage.c_str(), 2);

                /* Check if the new list of ports contains a port that did
                   not exist in the previous list. */
                for (std::vector<std::string>::iterator it = newPorts.begin();
                     it != newPorts.end();
                     ++it)
                {
                    if (std::find(oldPorts.begin(), oldPorts.end(), *it) == oldPorts.end())
                    {
                        fdevice = *it;
                        found = true;
                        break;
                    }
                }

                if (found)
                {
                    /* The new leonardo port appeared in the list.
                       Save it and go on. */
                    std::string leonardoDeviceMessage = "Found leonardo device on " + fdevice + " port";
                    CodebenderccAPI::debugMessage(leonardoDeviceMessage.c_str(), 2);
                    break;
                }

                oldPorts = newPorts;
                delay(250);
                elapsed_time += 250;

                /* If a certain ammount of time has gone by, and the initial
                   port is in the lost of ports, upload using this port */
                if (((os == "Widnows" && elapsed_time >= 500) || elapsed_time >=5000) &&
                    (std::find(oldPorts.begin(), oldPorts.end(), fdevice) != oldPorts.end()))
                {
                    std::string uploadingDeviceMessage = "Uploading using selected port: {" + fdevice +"}";
                    CodebenderccAPI::debugMessage(uploadingDeviceMessage.c_str(), 2);
                    break;
                }

                if (elapsed_time == 10000)
                {
                    notify("Could not auto-reset or detect a manual reset!");
                    flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-1));
                    return;
                }
            }
        }

		int retVal = 1;

    #if !defined  _WIN32 || _WIN64
		std::string command = "\"" + avrdude + "\"" + " -C\"" + avrdudeConf + "\"";
    #else
		std::string command = avrdude + " -C" + avrdudeConf;
    #endif
        if (CodebenderccAPI::checkDebug() && currentLevel >= 2)
			command += " -v -v -v -v";
		else
			command += " -V";
		command += " -P";
		command += (os == "Windows") ? "\\\\.\\" : "";
		command += fdevice + " -p" + mcu;
    #if defined  _WIN32 || _WIN64
		command += " -u -D -U flash:w:file.bin:a";
    #else
		command += " -u -D -U flash:w:\"" + binFile + "\":a";
    #endif
		command += " -c" + protocol + " -b" + speed + " -F";

		retVal = CodebenderccAPI::runAvrdude(command, false);
        _retVal = retVal;

		/* If the current board is leonardo, wait for a few seconds until
           the sketch actually takes control of the port */
		if (mcu == "atmega32u4")
        {
            delay(500);

            int timer = 0;
            while(timer < 2000)
            {
                std::vector<std::string> portVector;
                std::string ports = probeUSB();

                std::stringstream chk(ports);
                std::string tok;
                while (std::getline(chk, tok, ','))
                    portVector.push_back(tok);

                /* check if the bootloader has finished and the sketch has
                   taken control of the port */
                if (std::find(portVector.begin(), portVector.end(), fdevice) != portVector.end())
                {
                    delay(100);
                    timer += 100;

                    break;
                }

                delay(100);
                timer += 100;
            }
        }

		flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
    }
    catch (...)
    {
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflash exception",2);
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
    }

	CodebenderccAPI::debugMessage("CodebenderccAPI::doflash ended", 3);
}

void
CodebenderccAPI::doflashWithProgrammer(const std::string &device,
                                       const std::string &code,
                                       const std::string &maxsize,
                                       std::map<std::string, std::string> &programmerData,
                                       const std::string &mcu,
                                       const FB::JSObjectPtr &flash_callback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer", 3);

    std::string os = getPlugin().get()->getOS();

    try
    {
    #if !defined  _WIN32 || _WIN64
        chmod(avrdude.c_str(), S_IRWXU);
    #endif
        unsigned char buffer [150000];
        size_t size = base64_decode(code.c_str(), buffer, 150000);
        saveToBin(buffer, size);

        std::string fdevice = device;

        int retVal = 1;

        /* Create the first part of the command, which includes the
           programmer settings. */
        programmerData["mcu"] = mcu.c_str();
        programmerData["device"] = fdevice.c_str();

        std::string command = setProgrammerCommand(programmerData);

    #if defined  _WIN32 || _WIN64
        command += " -Uflash:w:file.bin:a";
    #else
        command += " -Uflash:w:\"" + binFile + "\":a";
    #endif

        /* Execute the upload command. */
        retVal = CodebenderccAPI::runAvrdude(command, false);
        _retVal = retVal;

        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
    }
    catch (...)
    {
        CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer exception", 2);
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
    }

    CodebenderccAPI::debugMessage("CodebenderccAPI::doflashWithProgrammer ended", 3);
}

void
CodebenderccAPI::doflashBootloader(const std::string &device,
                                   std::map<std::string, std::string> &programmerData,
                                   std::map<std::string, std::string> &bootloaderData,
                                   const std::string &mcu,
                                   const FB::JSObjectPtr &flash_callback)
{
	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader", 3);

	std::string os = getPlugin().get()->getOS();

	try
    {
    #if !defined  _WIN32 || _WIN64
        chmod(avrdude.c_str(), S_IRWXU);
    #endif

        std::string fdevice = device;
        int retVal = 1;

        /* Create the first part of the command, which includes the
           programmer settings. */
        programmerData["mcu"] = mcu.c_str();
        programmerData["device"] = fdevice.c_str();

        std::string programmerCommand = setProgrammerCommand(programmerData);

        /* The first part of the command is very likely to be used again
           when flashing the hex file. */
        std::string command = programmerCommand;

        /* Erase the chip, applying the proper values to the unlock bits
         * and high/low/extended fuses.
         *
         * Note: Values for high and low fuses MUST exist. The other values
         *       are optional, depending on the chip.
        */
        command += " -e";
        command += (bootloaderData["ulbits"] != "") ?
            " -Ulock:w:" + bootloaderData["ulbits"] + ":m" : "";
        command += (bootloaderData["efuses"] != "") ?
            " -Uefuse:w:" + bootloaderData["efuses"] + ":m" : "";
        command += " -Uhfuse:w:" + bootloaderData["hfuses"] + ":m" +
                   " -Ulfuse:w:" + bootloaderData["lfuses"] + ":m";

        retVal = CodebenderccAPI::runAvrdude(command, false);
        _retVal = retVal;

        /* If avrdude failed return the error code, else continue. */
        if (retVal == 0)
        {
            /* Apply a delay of one second. */
            delay(1000);

            /* Check if hex bootloader wsa sent from the server. If no
             * bootloader exists, an empty file is created. */
            std::ifstream file (hexFile.c_str(), std::ifstream::binary);
            if (file.is_open())
            {
                file.seekg(0, file.end);
                int length = file.tellg();

                if (length == 0)
                    file.close();
                else
                {
                    command = programmerCommand;
                #if defined  _WIN32 || _WIN64
                    command += " -Uflash:w:bootloader.hex:i";
                #else
                    command += " -Uflash:w:\"" + hexFile + "\":i";
                #endif
                    command += (bootloaderData["lbits"] != "") ?
                        " -Ulock:w:" + bootloaderData["lbits"] + ":m" : "";

                    retVal = CodebenderccAPI::runAvrdude(command, true);
                    _retVal = retVal;
                }
            }
        }

        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
    }
    catch (...)
    {
		CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader exception", 2);
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
	}

	CodebenderccAPI::debugMessage("CodebenderccAPI::doflashBootloader ended", 3);
}

const std::string
CodebenderccAPI::setProgrammerCommand(std::map<std::string, std::string>& programmerData)
{
	CodebenderccAPI::debugMessage("CodebenderccAPI::setProgrammerCommand", 3);

	std::string os = getPlugin().get()->getOS();

#if !defined  _WIN32 || _WIN64
		std::string command = "\"" + avrdude + "\"" + " -C\"" + avrdudeConf + "\"";
#else
		std::string command = avrdude + " -C" + avrdudeConf;
#endif

        /* Check if debugging is set to true. If the debug verbosity level
           is greater than 1, add verbosity flags to the avrdude command. */
        if (CodebenderccAPI::checkDebug() && currentLevel >= 2)
			command += " -v -v -v -v";
        else
			command += " -V";

		command += " -p" + programmerData["mcu"] +
                   " -c" + programmerData["protocol"];

		if (programmerData["communication"] == "usb")
			command += " -Pusb";
		else if (programmerData["communication"] == "serial")
        {
			command += " -P";
			command += (os == "Windows") ? "\\\\.\\" : "";
			command += programmerData["device"];

			if (programmerData["speed"] != "0")
				command += " -b" + programmerData["speed"];
		}

		if (programmerData["force"] == "true")
			command += " -F";
		if (programmerData["delay"] != "0")
			command += " -i" + programmerData["delay"];

	CodebenderccAPI::debugMessage("CodebenderccAPI::setProgrammerCommand ended", 3);
	return command;
}

int
CodebenderccAPI::runAvrdude(const std::string &command, bool append)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::runAvrdude", 3);

    int retval = 1;

#if defined  _WIN32 || _WIN64
    /* if on Windows, create a batch file and save the command in that file. */
    std::ofstream batchFd;

    try
    {
        batchFd.open(batchFile.c_str());
        batchFd << command;
        batchFd.close();
    }
    catch(...)
    {
        CodebenderccAPI::debugMessage("Failed to write command to batch file!", 1);
    }

    lastcommand = command;
    /* Call winExecAvrdude, which creates a new process, runs the batch
       file and gets all the ouput. */
    retval = winExecAvrdude(batchFile, append);
#else
    /* If on Unix-like system, simply make a system call, using the
       command as content and redirect the output to the output file. */
    std::string avrcommand = command;

    if (append)
        avrcommand += " 2>> \"" + outfile + "\"";
    else
        avrcommand += " 2> \"" + outfile + "\"";

    lastcommand = avrcommand;
    retval = CodebenderccAPI::system(avrcommand.c_str());
#endif

    /* Print the content of the output file, if debugging is on */
    if (CodebenderccAPI::checkDebug())
    {
        std::ifstream ifs(outfile.c_str());
        std::string content( (std::istreambuf_iterator<char>(ifs) ),
                             (std::istreambuf_iterator<char>()    ));
        CodebenderccAPI::debugMessage(content.c_str(),1);
    }
    CodebenderccAPI::debugMessage(lastcommand.c_str(),1);

    CodebenderccAPI::debugMessage("CodebenderccAPI::runAvrdude ended",3);
    return retval;
}

/**
 * Save the binary data to the binary file specified in the constructor.
*/
void
CodebenderccAPI::saveToBin(unsigned char *data, size_t size)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::saveToBin", 3);

    std::ofstream myfile;
    myfile.open(binFile.c_str(), std::fstream::binary);

    for (size_t i = 0; i < size; i++)
        myfile << data[i];

    myfile.close();

    CodebenderccAPI::debugMessage("CodebenderccAPI::saveToBin ended", 3);
}

/**
 * Save the hex data of the bootloader file to the hex file specified
 * in the constructor.
 */
void
CodebenderccAPI::saveToHex(const std::string &hexContent)
{
	CodebenderccAPI::debugMessage("CodebenderccAPI::saveToHex", 3);

	/* Save the content of the bootloader to a local hex file. */
	std::ofstream myfile;
    myfile.open(hexFile.c_str(), std::fstream::binary);

	myfile << hexContent;
	myfile.close();

	CodebenderccAPI::debugMessage("CodebenderccAPI::saveToHex ended", 3);
}

void
CodebenderccAPI::detectNewPort(const std::string &portString)
{
    std::vector<std::string> portVector;
    std::vector< std::string >::iterator externalIterator = portsList.begin();

    std::string mess;

    std::stringstream chk(portString);
    std::string tok;
    while (std::getline(chk, tok, ','))
        portVector.push_back(tok);

    std::vector< std::string >::iterator internalIterator = portVector.begin();

    while (externalIterator != portsList.end())
    {
        if (std::find(portVector.begin(), portVector.end(), *externalIterator) == portVector.end())
        {
            mess = "Device removed from port : " + *externalIterator;
            CodebenderccAPI::debugMessage(mess.c_str(), 1);
        }

        externalIterator++;
    }

    while (internalIterator != portVector.end())
    {
        if (std::find(portsList.begin(), portsList.end(), *internalIterator) == portsList.end())
        {
            mess = "Device added to port : " + *internalIterator;
            CodebenderccAPI::debugMessage(mess.c_str(),1);
        }

        internalIterator++;
    }

    portsList = portVector;
}

/**
 * decode base64 encoded data
 *
 * @param source the encoded data (zero terminated)
 *
 * @param target pointer to the target buffer
 *
 * @param targetlen length of the target buffer
 *
 * @return length of converted data on success, -1 otherwise
*/
size_t
CodebenderccAPI::base64_decode(const char *source,
                               unsigned char *target,
                               size_t targetlen)
{
    char *src, *tmpptr;
    char quadruple[4];
    unsigned char tmpresult[3];
    size_t tmplen = 3;
    int converted = 0;
    int i;

    /* concatenate '===' to the source to handle unpadded base64 data */
    src = (char *) malloc(strlen(source) + 5);
    if (src == NULL)
        return -1;
    strcpy(src, source);
    strcat(src, "====");
    tmpptr = src;

    /* convert as long as we get a full result */
    while (tmplen == 3)
    {
        /* get 4 characters to convert */
        for (i = 0; i < 4; i++)
        {
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
 * Determine the value of a base64 encoding character
 *
 * @param base64char the character of which the value is searched
 *
 * @return the value in case of success (0-63), -1 on failure
*/
int CodebenderccAPI::_base64_char_value(char base64char)
{
    /* A - Z */
	if (base64char >= 'A' && base64char <= 'Z')
        return base64char - 'A';
    /* a - z */
    if (base64char >= 'a' && base64char <= 'z')
        return base64char - 'a' + 26;
    /* 0 - 9 */
    if (base64char >= '0' && base64char <= '9')
        return base64char - '0' + 2 * 26;

    /* + */
    if (base64char == '+')
        return 2 * 26 + 10;
    /* / */
    if (base64char == '/')
        return 2 * 26 + 11;

    return -1;
}

/**
 * Decode a 4 char base64 encoded byte triple
 *
 * @param quadruple the 4 characters that should be decoded
 *
 * @param result the decoded data
 *
 * @return lenth of the result (1, 2 or 3), 0 on failure
*/
int
CodebenderccAPI::_base64_decode_triple(char quadruple[4],
                                       unsigned char *result)
{
    int i, triple_value, bytes_to_decode = 3, only_equals_yet = 1;
    int char_value[4];

    for (i = 0; i < 4; i++)
        char_value[i] = _base64_char_value(quadruple[i]);

    /* check if the characters are valid */
    for (i = 3; i >= 0; i--)
    {
        if (char_value[i] < 0)
        {
            if (only_equals_yet && quadruple[i] == '=')
            {
                /* we will ignore this character anyway, make it something
                   that does not break our calculations */
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

void
CodebenderccAPI::serialReader(const std::string &port,
                              const unsigned int &baudrate,
                              const FB::JSObjectPtr &callback)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::serialReader", 3);

    try
    {
        CodebenderccAPI::openPort(port, baudrate);

        int d;
        std::string rcvd;

        for (;;)
        {
            if (serialPort.isOpen())
                rcvd = "";

            rcvd = serialPort.read((size_t) 1);
            if (rcvd != "")
            {
                d = (int) rcvd[0];
                std::string characterMessage="Received character:";
                characterMessage.append(&rcvd[0]);
                CodebenderccAPI::debugMessage(characterMessage.c_str(), 2);

                callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(d));
            }
        }
    }
    catch (...)
    {
        CodebenderccAPI::debugMessage("CodebenderccAPI::serialReader loop interrupted", 1);
        notify("disconnect");
    }

    CodebenderccAPI::debugMessage("CodebenderccAPI::serialReader ended", 3);
}

std::string
CodebenderccAPI::exec(const char * cmd)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::exec", 3);

    std::string result = "";
#if !defined  _WIN32 || _WIN64
    FILE* pipe = CodebenderccAPI::popen(cmd, "r");
    if (!pipe)
        return "ERROR";

    char buffer[128];
    while (CodebenderccAPI::fgets(buffer, 128, pipe) != NULL)
        result += buffer;

    CodebenderccAPI::pclose(pipe);
#endif

    CodebenderccAPI::debugMessage("CodebenderccAPI::exec ended", 3);

    return result;
}

void
CodebenderccAPI::notify(const std::string &message)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::notify", 3);

    callback_->InvokeAsync("", FB::variant_list_of(shared_from_this())(message.c_str()));
}

int
CodebenderccAPI::programmerPrefs(const std::string &port,
                                 const std::string &programmerProtocol,
                                 const std::string &programmerSpeed,
                                 const std::string &programmerCommunication,
                                 const std::string &programmerForce,
                                 const std::string &programmerDelay,
                                 const std::string &mcu,
                                 std::map<std::string, std::string> &programmerData)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::programmerPrefs", 3);

    /**
     * Validate the programmer parameters
    */
    if (!validate_number(programmerSpeed))
        return -10;
    if (!validate_charnum(programmerProtocol))
        return -11;
    if (!validate_charnum(mcu))
        return -12;

    if (programmerProtocol != "usbtiny" && programmerProtocol != "dapa")
    {
        if (!validate_charnum(programmerCommunication))
            return -13;

        if (programmerCommunication == "serial")
        {
            if (!validate_device(port))
                return -14;
        }
    }

    if (!validate_charnum(programmerForce))
        return -15;
    if (!validate_number(programmerDelay))
        return -16;

    /**
     * Pass the programmer parameters to a map.
    */
    programmerData["protocol"] = programmerProtocol.c_str();
    programmerData["communication"] = programmerCommunication.c_str();
    programmerData["speed"] = programmerSpeed.c_str();
    programmerData["force"] = programmerForce.c_str();
    programmerData["delay"] = programmerDelay.c_str();

    CodebenderccAPI::debugMessage("CodebenderccAPI::programmerPrefs ended", 3);

    return 0;
}

int
CodebenderccAPI::bootloaderPrefs(const std::string& lFuses,
                                 const std::string& hFuses,
                                 const std::string& eFuses,
                                 const std::string& ulBits,
                                 const std::string& lBits,
                                 std::map<std::string, std::string>& data)
{
    CodebenderccAPI::debugMessage("CodebenderccAPI::bootloaderPrefs", 3);

    /**
     * Validate the bootloader parameters
    */
    if (lFuses == "" || !validate_hex(lFuses))
        return -17;
    if (hFuses == "" || !validate_hex(hFuses))
        return -18;
    if (eFuses != "" && !validate_hex(eFuses))
        return -19;
    if (ulBits != "" && !validate_hex(ulBits))
        return -20;
    if (lBits != "" && !validate_hex(lBits))
        return -21;

    /**
     * Pass the programmer parameters to a map.
    */
    data["hfuses"] = hFuses.c_str();
    data["lfuses"] = lFuses.c_str();
    data["efuses"] = eFuses.c_str();
    data["ulbits"] = ulBits.c_str();
    data["lbits"] = lBits.c_str();

    CodebenderccAPI::debugMessage("CodebenderccAPI::bootloaderPrefs ended", 3);

    return 0;
}

/**
 * validations
*/

bool
CodebenderccAPI::validate_hex(const std::string &input)
{
    return (input.compare(0, 2, "0x") == 0 &&
            input.size() > 2 &&
            input.size() <= 4 &&
            input.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos);
}

bool
CodebenderccAPI::validate_number(const std::string & input)
{
    try
    {
        boost::lexical_cast<double>(input);
        return true;
    }
    catch (boost::bad_lexical_cast &)
    {
        return false;
    }
}

bool
CodebenderccAPI::validate_device(const std::string &input)
{
    static const boost::regex acm("\\/dev\\/ttyACM[[:digit:]]+");
    static const boost::regex usb("\\/dev\\/ttyUSB[[:digit:]]+");
    static const boost::regex com("COM[[:digit:]]+");
    static const boost::regex cu("\\/dev\\/cu.[0-9a-zA-Z\\-]+");

    return boost::regex_match(input, acm) ||
           boost::regex_match(input, usb) ||
           boost::regex_match(input, com) ||
           boost::regex_match(input, cu);
}

bool
CodebenderccAPI::validate_code(const std::string &input)
{
	static const boost::regex base64("[0-9a-zA-Z+\\/=\n]+");

    return boost::regex_match(input, base64);
}

bool
CodebenderccAPI::validate_charnum(const std::string &input)
{
	static const boost::regex charnum("[0-9a-zA-Z]*");

    return boost::regex_match(input, charnum);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
    return NULL;
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
}

#if !defined(_WIN32) && !defined(_WIN64)
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
    return NULL;
}
#endif

#if !defined(_WIN32) && !defined(_WIN64)
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
}
#endif

/** TODO: we should return the result of WEXITSTATUS(rc) */
int
CodebenderccAPI::system(const char *command)
{
    std::string err_msg = "CodebenderccAPI::system() - ";
    int rc;

    rc = ::system(NULL);
    if (rc == 0) {
        CodebenderccAPI::debugMessage(
                (err_msg + "can not access the shell").c_str(), 3);
    }

    rc = ::system(command);
    if (rc == -1)
        err_msg += "Unknown error";
#if !defined(_WIN32) && !defined(_WIN64)
    else if (WEXITSTATUS(rc) == 127)
        err_msg += "/bin/sh could not be executed";
#endif
    else
        return rc;

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
    return rc;
}

#if defined(_WIN32) || defined(_WIN64)
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    rc = ::RegOpenKeyEx(hKey,
                        lpSubKey,
                        ulOptions,
                        samDesired,
                        phkResult);
    if (rc == ERROR_SUCCESS)
        return rc;

    /* TODO: Print error code description with FormatMessage.
     *
     * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724897%28v=vs.85%29.aspx
    */
    std::string err_msg = "CodebenderccAPI::RegOpenKeyEx() - Winerror.h error code: ";
    err_msg += boost::lexical_cast<std::string>(rc);

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
    return rc;
}

LONG
CodebenderccAPI::RegCloseKey(HKEY hKey)
{
    LONG rc;

    rc = ::RegCloseKey(hKey);
    if (rc == ERROR_SUCCESS)
        return rc;

    /* TODO: Print error code description with FormatMessage.
     *
     * http://msdn.microsoft.com/en-us/library/windows/desktop/ms724837%28v=vs.85%29.aspx
    */
    std::string err_msg = "CodebenderccAPI::RegCloseKey() - Winerror.h error code: ";
    err_msg += boost::lexical_cast<std::string>(rc);

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
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

    CodebenderccAPI::debugMessage(err_msg.c_str(), 3);
    return rc;
}

#endif
