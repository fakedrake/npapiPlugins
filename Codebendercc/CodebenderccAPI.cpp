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
    if (!grantedPermission) return "";

    if (!validate_device(device)) return -1;
    if (!validate_code(code)) return -2;
    if (!validate_number(maxsize)) return -3;
    if (!validate_number(speed)) return -4;
    if (!validate_charnum(protocol)) return -5;
    if (!validate_charnum(mcu)) return -6;
    perror("validated");

    boost::thread* t = new boost::thread(boost::bind(&CodebenderccAPI::doflash,
            this, device, code, maxsize, protocol, speed, mcu, flash_callback));

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

///////////////////////////////////////////////////////////////////////////////
/// @fn 
///
/// @brief  
///////////////////////////////////////////////////////////////////////////////

/**
 * Gets a reference to the plugin that was passed in when the object
 * was created.  If the plugin has already been released then this
 * will throw a FB::script_error that will be translated into a
 * javascript exception in the page.
 * @return 
 */
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

    std::string command = "cat /etc/group | grep $(ls -l " + port + " | cut -d ' ' -f 4) |  grep $USER";

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

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////PRIVATE////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CodebenderccAPI::doflash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr &flash_callback) {
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

void CodebenderccAPI::serialReader(const std::string &port, const unsigned int &baudrate, const FB::JSObjectPtr &callback) {
    try {
        serial = new boost::asio::serial_port(io, port);
        serial->set_option(boost::asio::serial_port_base::baud_rate(baudrate));
        char c;

        for (;;) {
            boost::asio::read(*serial, boost::asio::buffer(&c, 1));
            int d = (int) c;
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

bool CodebenderccAPI::validate_number(const std::string &input) {
    try {
        boost::lexical_cast<double>(input);
        return true;
    } catch (boost::bad_lexical_cast &) {
        return false;
    }
}

bool CodebenderccAPI::validate_device(const std::string &input) {
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

bool CodebenderccAPI::validate_code(const std::string &input) {
    static const boost::regex base64("[0-9a-zA-Z+\\/=\n]+");
    return boost::regex_match(input, base64);
}

bool CodebenderccAPI::validate_charnum(const std::string &input) {
    static const boost::regex charnum("[0-9a-zA-Z]+");
    return boost::regex_match(input, charnum);
}
