/**********************************************************\

  Auto-generated CodebenderccAPI.h

\**********************************************************/


#if defined _WIN32 || _WIN64
#include <SDKDDKVer.h>
#include "dirent.h"
#include <windows.h>
#include <tchar.h>
#else
#include <dirent.h>
#endif
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
//#include <boost/optional.hpp>
//#include <boost/weak_ptr.hpp>
#include <fstream>
//#include <iostream>
//#include <sstream>
//#include <stdio.h>
//#include <stdlib.h>
#include <sys/stat.h>
//#include <sys/types.h>

#include "BrowserHost.h"
#include "Codebendercc.h"
#include "DOM/Document.h"
#include "global/config.h"
#include "JSAPIAuto.h"
#include "JSObject.h"
#include "src/3rdParty/boost/boost/thread/detail/thread.hpp"
#include "SimpleStreamHelper.h"
#include "variant_list.h"


#ifndef H_CodebenderccAPI
#define H_CodebenderccAPI


class CodebenderccAPI : public FB::JSAPIAuto {
public:
    ////////////////////////////////////////////////////////////////////////////
    /// @fn CodebenderccAPI::CodebenderccAPI(const CodebenderccPtr& plugin, const FB::BrowserHostPtr host)
    ///
    /// @brief  Constructor for your JSAPI object.
    ///         You should register your methods, properties, and events
    ///         that should be accessible to Javascript from here.
    ///
    /// @see FB::JSAPIAuto::registerMethod
    /// @see FB::JSAPIAuto::registerProperty
    /// @see FB::JSAPIAuto::registerEvent
    ////////////////////////////////////////////////////////////////////////////

    CodebenderccAPI(const CodebenderccPtr& plugin, const FB::BrowserHostPtr& host) :
    m_plugin(plugin), m_host(host) {
        registerMethod("probeUSB", make_method(this, &CodebenderccAPI::probeUSB));
        registerMethod("download", make_method(this, &CodebenderccAPI::download));
        registerMethod("flash", make_method(this, &CodebenderccAPI::flash));
        registerMethod("getFlashResult", make_method(this, &CodebenderccAPI::getFlashResult));
        registerMethod("getLastCommand", make_method(this, &CodebenderccAPI::getLastCommand));
        registerMethod("validate_device", make_method(this, &CodebenderccAPI::validate_device));

        // Read-only property
        registerProperty("version",
                make_property(this,
                &CodebenderccAPI::get_version));
        std::string os = getPlugin().get()->getOS();
        std::string path = getPlugin().get()->getFSPath();
        path = path.substr(0, path.find_last_of("/\\") + 1);

        std::string arch = "32";
#ifdef __x86_64
        arch = "64";
#endif

        avrdude = path + os + ".avrdude";
        avrdudeConf = path + os + ".avrdude.conf";
        binFile = path + "file.bin";
        outfile = path + "out";

        libusb = path + "libusb0.dll";
        if (os == "Windows") {
            avrdude = path + "avrdude.exe";
        } else if (os == "X11") {
            avrdude = path + os + "." + arch + ".avrdude";
            avrdudeConf = path + os + "." + arch + ".avrdude.conf";
        } 
    }

    /**
     * Destructor.
     */
    virtual ~CodebenderccAPI() {
    };

    /**
     * 
     * @return 
     */
    CodebenderccPtr getPlugin();

    /**
     * Gets the plugin version.
     * @return the version string.
     */
    std::string get_version();

    /**
     * Deprecated.
     * @return 
     */
    FB::variant download();
    /**
     * 
     * @param url
     * @param destination
     */
    void getURL(const std::string& url, const std::string& destination);
    /**
     * 
     * @param device
     * @param code
     * @param maxsize
     * @param protocol
     * @param speed
     * @param mcu
     * @return 
     */
    FB::variant flash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu);
    /**
     * Checks for all available USB Arduino devices.
     * @return a comma separated list of the detected devices.
     */
    FB::variant probeUSB();
    /**
     * Returns the avrdude 's output.
     * @return the output recorded from avrdude.
     */
    FB::variant getFlashResult();
    /**
     * The last avrdude command executed.
     * @return 
     */
    FB::variant getLastCommand();

    /**
     * Validate Device Name.
     * Devices in Linux are /dev/tty{USB??,ACM??}.
     * Devices in Mac are /dev/cu.{}.
     * Devices in Windows are COM??.
     * 
     * @param input the device string.
     * @return true if valid, false else.
     */
    bool validate_device(const std::string &input);


private:
    /**
     * Check if the file exists.
     * @param filename the file to check.
     * @return true if it exists.
     */
    bool fileExists(const std::string& filename);
    /**
     * 
     * @param success
     * @param headers
     * @param data
     * @param size
     * @param destination
     */
    void getURLCallback(bool success, const FB::HeaderMap& headers, const boost::shared_array<uint8_t>& data, const size_t size, const std::string& destination);
    /**
     * 
     * @param source
     * @param target
     * @param targetlen
     * @return 
     */
    size_t base64_decode(const char *source, unsigned char *target, size_t targetlen);
    /**
     * 
     * @param base64char
     * @return 
     */
    int _base64_char_value(char base64char);
    /**
     * 
     * @param quadruple
     * @param result
     * @return 
     */
    int _base64_decode_triple(char quadruple[4], unsigned char *result);
    /**
     * Saves a Binary file to disk.
     * @param data the data to write.
     * @param size the size of the buffer.
     */
    void saveToBin(unsigned char * data, size_t size);

    /**
     * Validate a number string.
     * @param input the string to validate.
     * @return true if the string contains a number, false else.
     */
    bool validate_number(const std::string &input);
    /**
     * Validate a base64 file string.
     * @param input the code for the Arduino in base64 format.
     * @return true if the string is valid, false else.
     */
    bool validate_code(const std::string &input);
    /**
     * Validate a string that contains digits and characters.
     * @param input the string to validate.
     * @return true if valid,false else.
     */
    bool validate_charnum(const std::string &input);

    /**
     */
    CodebenderccWeakPtr m_plugin;
    /**
     */
    FB::BrowserHostPtr m_host;
    /**
     */
    std::string avrdude, avrdudeConf, binFile, outfile;
    /**
     */
    std::string libusb;
    /**
     */
    std::string lastcommand;
};

#endif // H_CodebenderccAPI

