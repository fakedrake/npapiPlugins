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
FB::variant CodebenderccAPI::tftpUpload(const FB::JSObjectPtr & cback, const std::string& ip, const std::string& code, const std::string& port1, const std::string& passphrase, const std::string& port2) {
    if (!validate_code(code)) return -2;
    if (!validate_charnum(passphrase)) return -4;

    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::doTftpUpload, this, cback, ip, code, port1, passphrase, port2));

    return 0;
}
#endif

FB::variant CodebenderccAPI::flash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr &flash_callback) {
    if (!grantedPermission) return "";

    if (!validate_device(device)) return -1;
    if (!validate_code(code)) return -2;
    if (!validate_number(maxsize)) return -3;
    if (!validate_number(speed)) return -4;
    if (!validate_charnum(protocol)) return -5;
    if (!validate_charnum(mcu)) return -6;
    perror("validated");

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
    if (!grantedPermission) return false;

    callback_ = callback;
    return true;
}

#if defined _WIN32||_WIN64

std::string CodebenderccAPI::probeUSB() {
    if (!grantedPermission) return "";

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
    if (!grantedPermission) return "";

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
    if (!grantedPermission) return "";

    return lastcommand;
}

FB::variant CodebenderccAPI::getFlashResult() {
    if (!grantedPermission) return "";

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
    if (!grantedPermission) return;

    std::string mess = message + '\n';
    if (serial != NULL) {
        perror("writing");
        boost::asio::write(*serial, boost::asio::buffer(mess.c_str(), mess.size()));
    } else {
        perror("null");
    }
}

FB::variant CodebenderccAPI::disconnect() {
    if (!grantedPermission) return "";

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
    if (!grantedPermission) return "";

#if !defined  _WIN32 || _WIN64	
    if (!validate_device(port)) return "";
    if (getPlugin().get()->getOS() != "X11") return "";

    std::string command = "groups | grep $(ls -l " + port + " | cut -d ' ' -f 4)";

    std::string result = exec(command.c_str());
    if (result == "") {
        command = "ls -l " + port + " | cut -d ' ' -f 4";

        return exec(command.c_str());
    }
#endif
    return "";
}

bool CodebenderccAPI::serialRead(const std::string &port, const std::string &baudrate, const FB::JSObjectPtr &callback) {
    if (!grantedPermission) return false;

    std::string message = "connecting at ";
    message += baudrate;

    callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(message));

    unsigned int brate = boost::lexical_cast<unsigned int, std::string > (baudrate);
    doclose = false;

    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::serialReader, this, port, brate, callback));
    return true; // the thread is started
}

FB::variant CodebenderccAPI::installDrivers(int version) {
    if (!grantedPermission) return -1;
    //#if defined _WIN32||_WIN64
    if (getPlugin().get()->getOS() == "Windows") {
        std::string path = getPlugin().get()->getFSPath();
        path = path.substr(0, path.find_last_of("/\\") + 1);

        std::string adminVBS = "Set UAC = CreateObject(\"Shell.Application\")\n UAC.ShellExecute \"" + path + "driverCopy.bat\", \"\", \"\", \"runas\", 1 \n";

        std::ofstream myfile;
        myfile.open((path + "admin.vbs").c_str(), std::fstream::out);
        myfile << adminVBS;
        myfile.close();

        std::string driverCopyBAT = "xcopy /sy \"" + path + "drivers\" \\Windows\\inf";

        std::ofstream myfile1;
        myfile1.open((path + "driverCopy.bat").c_str(), std::fstream::out);
        myfile1 << driverCopyBAT;
        myfile1.close();


        std::string command = "\"" + path + "admin.vbs\"";
        if (version == 0) {
            command = "\"" + path + "driverCopy.bat\"";
        }

        command = "\"" + command + "\"";
        int retVal = system(command.c_str());
        return 0;
    }
    //#endif
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
        notify("Error 1: Could not create initial authorization.");
        return "Error 1: Could not create initial authorization. " + status;
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
        notify("Error 2: Could not copy authorization rights.");
        return "Error 2: Could not copy authorization rights. " + status;
    }

    notify("Installing Drivers...");
    char buffer[1024];
    int bytesRead;
    std::string output = "output ";

    char *tool = "/usr/sbin/installer";
    std::string driverPath = path + "/FTDIUSBSerialDriver_10_4_10_5_10_6_10_7.mpkg";
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
        notify("Error 3: Failed to execute call with privileges.");
        return "Error 3: Failed to execute call with privileges." + status;
    }

    // The only way to guarantee that a credential acquired when you
    // request a right is not shared with other authorization instances is
    // to destroy the credential.  To do so, call the AuthorizationFree
    // function with the flag kAuthorizationFlagDestroyRights.
    // http://developer.apple.com/documentation/Security/Conceptual/authorization_concepts/02authconcepts/chapter_2_section_7.html
    status = AuthorizationFree(authorizationRef, kAuthorizationFlagDestroyRights);
    notify("Drivers Installed\n" + output);
    return "Drivers Installed";
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
        std::string url = "http://" + ip + ":" + port2 + "/" + passphrase + "/reset";
		FB::SimpleStreamHelper::AsyncGet(m_host,FB::URI::fromString(url),
		    boost::bind(&CodebenderccAPI::getURLCallback, this, _1, _2, _3, _4),false);
    }
#endif

    //write file to disk
    unsigned char buffer [150000];
    size_t size = base64_decode(code.c_str(), buffer, 150000);
    saveToBin(buffer, size);

    //do the upload if nothing has gone sideways
    char *s2 = new char[ip.size() + 1];
    strcpy(s2, ip.c_str());
    cback->InvokeAsync("", FB::variant_list_of(shared_from_this())("launching client "));
    cback->InvokeAsync("", FB::variant_list_of(shared_from_this())(s2));

    //connect with tftp
    TFTPClient client = TFTPClient(s2, boost::lexical_cast<int>(port1));
    client.connectToServer();
    //create the filenames
    char remoteFile[9];
    strcpy(remoteFile, "file.bin");
    char *localFile= new char[binFile.size() + 1];
    strcpy(localFile, binFile.c_str());
    strcpy(localFile, remoteFile);
    //actually send the file
    client.sendFile(localFile, remoteFile);
}

void CodebenderccAPI::getURLCallback(bool success, const FB::HeaderMap& headers, const boost::shared_array<uint8_t>& data, const size_t size) {
}

#endif
	

void CodebenderccAPI::doflash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr & flash_callback) {
    if (!grantedPermission) return;
    perror("doflash");
#if !defined  _WIN32 || _WIN64	
    chmod(avrdude.c_str(), S_IRWXU);
#endif

    if (mcu == "atmega32u4") {
        notify("Trying Arduino Leonardo auto-reset. If it does not reset automatically please reset the Arduino manualy!");
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
#if !defined  _WIN32 || _WIN64	
                usleep(2000000);
#else
                Sleep(2000);
#endif
                mySerial.close();
            } catch (...) {
            }
        }

#if !defined  _WIN32 || _WIN64	
        usleep(500000);
#else
        Sleep(500);
#endif

        std::string oldPorts = probeUSB();
        perror(oldPorts.c_str());

        for (int i = 0; i < 20; i++) {
#if !defined  _WIN32 || _WIN64	
            sleep(1);
#else
            Sleep(1000);
#endif
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

    int retVal = system(command.c_str());
    perror(command.c_str());
    flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
}
#ifdef ENABLE_DIGISPARK
void CodebenderccAPI::doflashDigispark(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr & flash_callback) {
    if (!grantedPermission) return;
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

    notify("Please plug in the device (timeout in 20 seconds)...");

    int retVal = system(command.c_str());
    perror(command.c_str());
    if (retVal == 0) {
        notify("Digispark Flashed");
    } else if (retVal == 256) {
        notify("Flashed Failed. Please Retry...");
    } else if (retVal == 34304) {
        notify("Please unplug your Digispark and try flashing again...");
    } else {
        flash_callback->InvokeAsync("", FB::variant_list_of(shared_from_this())(retVal));
    }
}
#endif

/**
 * Save the binary data to the binary file specified in the constructor.
 */
void CodebenderccAPI::saveToBin(unsigned char * data, size_t size) {
    if (!grantedPermission) return;

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

void CodebenderccAPI::notify(const std::string & message) {

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
