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
#ifdef ENABLE_TFTP

FB::variant CodebenderccAPI::tftpUpload(const FB::JSObjectPtr & cback, const std::string& address, const std::string& code) {
    if (!validate_code(code)) return -2;
    //    if (!validate_charnum(passphrase)) return -4;
    std::string ip, port1, passphrase, port2 = "69";
    int foundSemicolon = address.find(":");
    int foundSlash = address.find("/");

    //getIP
    if (foundSemicolon != std::string::npos) {
        ip = std::string(address.c_str(), 0, foundSemicolon);
    } else if (foundSlash != std::string::npos) {
        ip = std::string(address.c_str(), 0, foundSlash);
    } else {
        ip = address;
    }
    //getPort
    if (foundSemicolon != std::string::npos) {
        if (foundSlash != std::string::npos) {
            port1 = std::string(address.c_str(), foundSemicolon + 1, foundSlash - foundSemicolon - 1);
        } else {
            port1 = std::string(address.c_str(), foundSemicolon + 1, address.length());
        }
    } else {
        port1 = "80";
    }
    //getPassphrase
    if (foundSlash != std::string::npos) {
        passphrase = std::string(address.c_str(), foundSlash + 1, address.length());
    } else {
        passphrase = "";
    }
    tftp_callback_ = cback;

    //    notify("ip:'" + ip + "',port:'" + port1 + "',phrase:'" + passphrase + "',p2:'" + port2 + "'");
    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::doTftpUpload, this, cback, ip, code, port1, passphrase, port2));

    return 0;
}
#endif

FB::variant CodebenderccAPI::flash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr &flash_callback) {
    if (!validate_device(device)) return -1;
    if (!validate_code(code)) return -2;
    if (!validate_number(maxsize)) return -3;
    if (!validate_number(speed)) return -4;
    if (!validate_charnum(protocol)) return -5;
    if (!validate_charnum(mcu)) return -6;

    if (protocol == "digispark") {
#ifdef ENABLE_DIGISPARK
        boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::doflashDigispark,
                this, device, code, maxsize, protocol, speed, mcu, flash_callback));
#endif
    } else {
        boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::doflash,
                this, device, code, maxsize, protocol, speed, mcu, flash_callback));
    }
    return 0;
}

bool CodebenderccAPI::setCallback(const FB::JSObjectPtr &callback) {

    callback_ = callback;
    return true;
}

#if defined _WIN32||_WIN64

std::string CodebenderccAPI::probeUSB() {

    HANDLE hCom;
    std::string ports = "";

    for (int i = 1; i < 150; i++) {
        //std::string port ="COM11";   //"\\\\.\\"

        TCHAR pcCommPort [32]; //  Most systems have a COM1 port
        swprintf(pcCommPort, L"\\\\.\\COM%d", i);
        //_tcscpy(pcCommPort ,L(port.c_str()));
        //  Open a handle to the specified com port.
        hCom = CreateFile(pcCommPort,
                GENERIC_READ,
                0, //  must be opened with exclusive-access
                NULL, //  default security attributes
                OPEN_EXISTING, //  must use OPEN_EXISTING
                0, //  not overlapped I/O
                NULL); //  hTemplate must be NULL for comm devices

        if (hCom == INVALID_HANDLE_VALUE) {
        } else {
            CloseHandle(hCom);
            ports.append("COM");
            ports.append(boost::lexical_cast<std::string, int>(i));
            ports.append(",");
        }
    }

    return ports;
}
#else

std::string CodebenderccAPI::probeUSB() {

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
    } else
        perror("Couldn't open the directory");

    return dirs;
}
#endif

CodebenderccPtr CodebenderccAPI::getPlugin() {

    CodebenderccPtr plugin(m_plugin.lock());
    if (!plugin) {
        throw FB::script_error("The plugin is invalid");
    }
    return plugin;
}

// Read-only property version

std::string CodebenderccAPI::get_version() {
    return FBSTRING_PLUGIN_VERSION;
}

FB::variant CodebenderccAPI::getLastCommand() {

    return lastcommand;
}

FB::variant CodebenderccAPI::getFlashResult() {

    FILE *pFile;
    pFile = fopen(outfile.c_str(), "r");
    char buffer[128];
    std::string result = "";
    while (!feof(pFile)) {
        if (fgets(buffer, 128, pFile) != NULL)
            result += buffer;
    }
    fclose(pFile);
    return result;
}

void CodebenderccAPI::serialWrite(const std::string & message) {

    std::string mess = message;
    if (serial != NULL) {
        perror("writing");
        boost::asio::write(*serial, boost::asio::buffer(mess.c_str(), mess.size()));
    } else {
        perror("null");
    }
}

FB::variant CodebenderccAPI::disconnect() {

    if (serial == NULL)return 1;
    try {
        doclose = true;
        serial->close();
        serial = NULL;
    } catch (...) {
    }
    return 1;
}

FB::variant CodebenderccAPI::checkPermissions(const std::string & port) {

#if !defined  _WIN32 || _WIN64	
    if (!validate_device(port)) return "";
    if (getPlugin().get()->getOS() != "X11") return "";

    std::string command = "groups | grep $(ls -l " + port + " | cut -d ' ' -f 4)";

    std::string result = exec(command.c_str());
    if (result == "") {
        command = "ls -l " + port + " | cut -d ' ' -f 4";

        return exec(command.c_str());
    }
    return "";
#else
    BOOL bIsProtected = false;
    HRESULT hr = IEIsProtectedModeProcess(&bIsProtected);
    return bIsProtected;
#endif


}
#if defined _WIN32 || _WIN64

int CodebenderccAPI::runme(const std::string & command, const std::string & arguments) {

    DWORD dwExitCode = -1;
    std::wstring stemp = s2ws(command);
    LPCWSTR params = stemp.c_str();
    std::wstring sargs = s2ws(arguments);
    LPCWSTR args = sargs.c_str();

    SHELLEXECUTEINFO ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof (SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = L"open";
    ShExecInfo.lpFile = params;
    ShExecInfo.lpParameters = args;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);

    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

    GetExitCodeProcess(ShExecInfo.hProcess, &dwExitCode);
    return dwExitCode;
}
#endif

bool CodebenderccAPI::serialRead(const std::string &port, const std::string &baudrate, const FB::JSObjectPtr &callback) {

    std::string message = "connecting at ";
    message += baudrate;

    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(message));

    unsigned int brate = boost::lexical_cast<unsigned int, std::string > (baudrate);
    doclose = false;

    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::serialReader, this, port, brate, callback));
    return true; // the thread is started
}

bool CodebenderccAPI::checkForDrivers(const std::string& driver) {
#if defined _WIN32||_WIN64
    if (driver == "") {
        bool result = true;
        bool newDrivers = checkForDrivers("arduino.inf") && checkForDrivers("arduino.cat");
        if (!newDrivers) {
            result = result && checkForDrivers("Arduino USBSerial.inf");
            result = result && checkForDrivers("Arduino UNO.inf");
            result = result && checkForDrivers("Arduino UNO REV3.inf");
            result = result && checkForDrivers("Arduino Micro.inf");
            result = result && checkForDrivers("Arduino Mega ADK.inf");
            result = result && checkForDrivers("Arduino Mega ADK REV3.inf");
            result = result && checkForDrivers("Arduino MEGA 2560.inf");
            result = result && checkForDrivers("Arduino MEGA 2560 REV3.inf");
            result = result && checkForDrivers("Arduino Leonardo.inf");
        }
        result = result && checkForDrivers("Digispark_Bootloader.inf");
        result = result && checkForDrivers("Digispark_Bootloader.cat");
        result = result && checkForDrivers("x86/libusb0.sys");
        result = result && checkForDrivers("x86/libusb0_x86.dll");
        result = result && checkForDrivers("x86/libusbK_x86.dll");
        result = result && checkForDrivers("amd64/libusb0.dll");
        result = result && checkForDrivers("amd64/libusb0.sys");
        result = result && checkForDrivers("amd64/libusbK.dll");

        return result;
    } else {
        return fs::exists("\\Windows\\inf\\" + driver);
    }
#endif
#ifdef __APPLE__
    return fs::exists(LOCATION_DRIVERS_ARDUINO_OSX);
#endif
}

FB::variant CodebenderccAPI::installDrivers(int mode) {

#if defined _WIN32||_WIN64
    if (getPlugin().get()->getOS() == "Windows") {
        notify(MSG_DRIVER_INSTALL);

        fs::remove(path + "\\adminrun");

        std::string adminVBS = "Set UAC = CreateObject(\"Shell.Application\")\n UAC.ShellExecute \"" + path + "driverCopy.bat\", \"\", \"\", \"runas\", 1 \n";

        std::ofstream myfile;
        myfile.open((path + "admin.vbs").c_str(), std::fstream::out);
        myfile << adminVBS;
        myfile.close();

        std::string driverCopyBAT = "xcopy /sy \"" + path + "drivers\" \\Windows\\inf && echo test>\"" + path + "adminrun\" ";

        std::ofstream myfile1;
        myfile1.open((path + "driverCopy.bat").c_str(), std::fstream::out);
        myfile1 << driverCopyBAT;
        myfile1.close();

        int retVal = -1;
        std::string command = "\"" + path + "admin.vbs\"";
        if (mode == 0) {
            command = "\"" + path + "driverCopy.bat\"";
            myfile.open(path + "\\adminrun", std::fstream::out);
            myfile.close();
        }
        retVal = system(command.c_str());

        //command = "\"" + command + "\"";
        //int retVal = system(command.c_str());

        bool result = true;

        delay(5000);

        result = checkForDrivers("");

        if (!fs::exists(path + "\\adminrun")) {
            notify(MSG_DRIVER_ALLOW);
            return result;
        }

        if (result) {
            notify(MSG_DRIVER_INSTALLED);
        } else {
            notify(MSG_DRIVER_NOT_INSTALLED);
        }
		
        return result;
    }
#endif
#ifdef __APPLE__

    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::installDriversMac, this));
    return "Attempting to install Drivers on Mac";
#endif
    return 0;
}


//std::string driverPath = path+ "/FTDIUSBSerialDriver_10_4_10_5_10_6_10_7.mpkg";
//char *args[] = {"-pkg", driverPath.c_str(), "-target", "/"};



#ifdef __APPLE__

void CodebenderccAPI::installDriversMac() {
    notify(MSG_DRIVERS_DECOMPRESS);

    std::string command = "unzip \"" + path + "ftdi.zip" + "\" -d \"" + path + "\"";
    int retVal = system(command.c_str());


    // Create authorization reference
    OSStatus status;
    AuthorizationRef authorizationRef;

    // AuthorizationCreate and pass NULL as the initial
    // AuthorizationRights set so that the AuthorizationRef gets created
    // successfully, and then later call AuthorizationCopyRights to
    // determine or extend the allowable rights.
    // http://developer.apple.com/qa/qa2001/qa1172.html
    status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authorizationRef);
    if (status != errAuthorizationSuccess) {
        notify(MSG_DRIVER_ERROR_AUTHORIZE_1);
        return;// "Error 1: Could not create initial authorization. " + status;
    }


    // kAuthorizationRightExecute == "system.privilege.admin"
    AuthorizationItem right = {kAuthorizationRightExecute, 0, NULL, 0};
    AuthorizationRights rights = {1, &right};
    AuthorizationFlags flags = kAuthorizationFlagDefaults |
            kAuthorizationFlagInteractionAllowed |
            kAuthorizationFlagPreAuthorize |
            kAuthorizationFlagExtendRights;


    // Call AuthorizationCopyRights to determine or extend the allowable rights.
    status = AuthorizationCopyRights(authorizationRef, &rights, NULL, flags, NULL);
    if (status != errAuthorizationSuccess) {
        notify(MSG_DRIVER_ERROR_AUTHORIZE_2);
        return;// "Error 2: Could not copy authorization rights. " + status;
    }

    notify(MSG_DRIVER_INSTALL);
    char buffer[1024];
    int bytesRead;
    std::string output = "";

    char *tool = "/usr/sbin/installer";
    std::string driverPath = path + "ftdi.mpkg";
    //char *args[] = {NULL}; //{"-pkg", const_cast<char *> (driverPath.c_str()), "-target", "/"};
    char *args[] = {"-pkg", const_cast<char *> (driverPath.c_str()), "-target", "/", NULL};
    FILE *pipe = NULL;

    status = AuthorizationExecuteWithPrivileges(authorizationRef, tool, kAuthorizationFlagDefaults, args, &pipe);

    /* Just pipe processes' stdout to our stdout for now; hopefully can add stdin pipe later as well */
    for (;;) {
        bytesRead = fread(buffer, sizeof (char), 1024, pipe);
        if (bytesRead < 1) break;
        output += buffer;
    }


    if (status != errAuthorizationSuccess) {
        notify(MSG_DRIVER_ERROR_AUTHORIZE_3);
        return;// "Error 3: Failed to execute call with privileges." + status;
    }

    // The only way to guarantee that a credential acquired when you
    // request a right is not shared with other authorization instances is
    // to destroy the credential.  To do so, call the AuthorizationFree
    // function with the flag kAuthorizationFlagDestroyRights.
    // http://developer.apple.com/documentation/Security/Conceptual/authorization_concepts/02authconcepts/chapter_2_section_7.html
    status = AuthorizationFree(authorizationRef, kAuthorizationFlagDestroyRights);
    if (checkForDrivers("")) {
        notify("Drivers Installed Successfully.<span style='display:none;'>Output: " + output + "</span>");
        return;// "Drivers Installed Successfully.";
    } else {
        notify("There was a problem during the Installation.<span style='display:none;'>Output: " + output + "</span>");
        return;// "There was a problem during the Installation.";
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////PRIVATE////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_TFTP

void CodebenderccAPI::doTftpUpload(const FB::JSObjectPtr & cback, const std::string& ip, const std::string& code, const std::string& port1, const std::string& passphrase, const std::string& port2) {
#ifdef ENABLE_TFTP_AUTO_RESET
    //reset if needed
    if (passphrase != "") {
        //buld the reset url
        std::string url = "http://" + ip + ":" + port1 + "/" + passphrase + "/reset";
        FB::SimpleStreamHelper::AsyncGet(m_host, FB::URI::fromString(url),
                boost::bind(&CodebenderccAPI::getURLCallback, this, _1, _2, _3, _4), false);
    }
#endif

    //write file to disk
    unsigned char buffer [150000];
    size_t size = base64_decode(code.c_str(), buffer, 150000);
    saveToBin(buffer, size);

    //do the upload if nothing has gone sideways
    //    char *s2 = new char[ip.size() + 1];
    //    strcpy(s2, ip.c_str());
    //    cback->InvokeAsync("", FB::variant_list_of(shared_from_this())("launching client "));
    //    cback->InvokeAsync("", FB::variant_list_of(shared_from_this())(s2));
    delay(5000);

    //connect with tftp
    TFTPClient<CodebenderccAPI> client = TFTPClient<CodebenderccAPI>(ip.c_str(), 69, this);
    int connectionResult = client.connectToServer();

    packets = size / client.packetSize() + 1;

    if (connectionResult == 1) {
        //create the filenames
        char remoteFile[9];
        strcpy(remoteFile, "file.bin");
        char *localFile = new char[binFile.size() + 1];
        strcpy(localFile, binFile.c_str());
        notify(MSG_TFTP_STARTING + ip);
        //actually send the file
        int res = client.sendFile(localFile, remoteFile);
        if (res == 0) {
            notify(MSG_FILE_UPLOADED);
        } else {
            if (res == 1) {
                notify(MSG_TFTP_ERROR_LOCAL_FILE);
            } else {
                notify(MSG_TFTP_ERROR_TIMEOUT);
            }
        }
    } else {
        notify(MSG_TFTP_ERROR_CONNECTION);
    }
}

void CodebenderccAPI::getURLCallback(bool success, const FB::HeaderMap& headers, const boost::shared_array<uint8_t>& data, const size_t size) {
    std::string dstr(reinterpret_cast<const char*> (data.get()), size);
    notify(dstr);
}

#endif

void CodebenderccAPI::doflash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr & flash_callback) {
    try {
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
            {
                try {
                    boost::asio::serial_port mySerial(io, fdevice);
                    mySerial.set_option(boost::asio::serial_port_base::baud_rate(1200));
                    delay(2000);
                    mySerial.close();
                } catch (...) {
                }
            }

            delay(500);

            std::string oldPorts = probeUSB();
            perror(oldPorts.c_str());

            for (int i = 0; i < 20; i++) {
                delay(1000);

                std::vector<std::string> newPorts;
                std::string newports = probeUSB();
                perror(newports.c_str());
                std::stringstream ss(newports);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    newPorts.push_back(item);
                }

                for (std::vector<std::string>::iterator it = newPorts.begin(); it != newPorts.end(); ++it) {
                    if (oldPorts.find(*it) == std::string::npos) {
                        fdevice = *it;
                        i = 20;
                        break;
                    }
                }
                //            boost::posix_time::ptime t2 = boost::posix_time::second_clock::local_time();

                if (i == 19) {
                    notify("Could not auto-reset or detect a manual reset!");
                    flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(-1));
                    return;
                }
            }
        }

        std::string command = "\"" + avrdude + "\""
                + " -C\"" + avrdudeConf + "\""
                + " -P" + fdevice
                + " -p" + mcu
                + " -u -U flash:w:\"" + binFile + "\":a"
                + " -c" + protocol
                + " -b" + speed
                + " -F 2> "
                + "\"" + outfile + "\"";

        if (getPlugin().get()->getOS() == "Windows") {

            command = "\"" + command + "\"";
        }

        lastcommand = command;

        int retVal = 1;

#if !defined  _WIN32 || _WIN64	 
        retVal = system(command.c_str());
#else

        command = " -C\"" + avrdudeConf + "\""
                + " -P" + fdevice
                + " -p" + mcu
                + " -u -U flash:w:\"" + binFile + "\":a"
                + " -c" + protocol
                + " -b" + speed
                + " -F ";
        retVal = runme(avrdude, command);

#endif

        _retVal = retVal;
        perror(command.c_str());
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
    } catch (...) {
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
    }
}
#ifdef ENABLE_DIGISPARK

void CodebenderccAPI::doflashDigispark(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr & flash_callback) {
    try {
        perror("doflashdigispark");
#if !defined  _WIN32 || _WIN64	
        chmod(digispark.c_str(), S_IRWXU);
#endif

        unsigned char buffer [150000];
        size_t size = base64_decode(code.c_str(), buffer, 150000);
        saveToBin(buffer, size);

        std::string fdevice = device;

        std::string command = "\"" + digispark + "\""
                + " --run --timeout 20 --type raw "
                + "\"" + binFile + "\" > \"" + outfile + "\"";

        if (getPlugin().get()->getOS() == "Windows") {

            command = "\"" + command + "\"";
        }

        lastcommand = command;

        notify(MSG_DIGISPARK_TIMEOUT);
        int retVal = -1;
#if !defined  _WIN32 || _WIN64	
        retVal = system(command.c_str());
#else
        command = " --run --timeout 20 --type raw \"" + binFile + "\"";

        retVal = runme(digispark, command.c_str());
#endif
        perror(command.c_str());
        if (retVal == 0) {
            notify(MSG_DIGISPARK_FLASHED);
        } else if (retVal == 256) {
            notify(MSG_DIGISPARK_ERROR);
        } else if (retVal == 34304) {
            notify(MSG_DIGISPARK_ERROR_WAS_PLUGED);
        } else {
            flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
        }
    } catch (...) {
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(9001));
    }
}
#endif

/**
 * Save the binary data to the binary file specified in the constructor.
 */
void CodebenderccAPI::saveToBin(unsigned char * data, size_t size) {

    std::ofstream myfile;
    myfile.open(binFile.c_str(), std::fstream::binary);
    for (size_t i = 0; i < size; i++) {

        myfile << data[i];
    }
    myfile.close();
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
    try {
        serial = new boost::asio::serial_port(io, port);
        serial->set_option(boost::asio::serial_port_base::baud_rate(baudrate));
        char c;
        int d;

        for (;;) {
            if (serial == NULL) break;
            boost::asio::read(*serial, boost::asio::buffer(&c, 1));
            d = (int) c;
            callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(d));
        }
    } catch (...) {

        serial = NULL;
        notify("disconnect");
    }
}

std::string CodebenderccAPI::exec(const char * cmd) {
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
    return result;
}

void CodebenderccAPI::notify(const std::string &message) {
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
