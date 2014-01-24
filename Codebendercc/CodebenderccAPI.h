/*! \mainpage Codebender Browser NPAPI Plugin Documentation Pages
 *
 * \section intro_sec Introduction
 *
 * These pages give a better understanding to the people that want to work on or use the Codebender Browser plugin.\n  
 *
 * \section install_sec Installation
 * The generated library file should be placed in the suitable directory for each platform/browser.
 * 
 * \author Dimitrios Amaxilatis {d.amaxilatis at gmail}
 * 
 * \todo fix the error messages add full-stops and rest
 * @see http://codebender.cc
 * @see http://www.firebreath.org/display/documentation/FireBreath+Home
 */


#if defined _WIN32 || _WIN64
#define MAX_KEY_LENGTH 255
#define WIN32_LEAN_AND_MEAN 
#include <SDKDDKVer.h>
#include "dirent.h"
#include <windows.h>
#include <tchar.h>
#include <Shellapi.h>
#include <Tchar.h>
#include <Iepmapi.h>

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#else
#include <dirent.h>
#endif
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/date_time.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/array.hpp>
//#include <boost/optional.hpp>
//#include <boost/weak_ptr.hpp>
//used to check for the drivers
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

#include <fstream>
#include <vector>
//#include <iostream>
//#include <sstream>
//#include <stdio.h>
//#include <stdlib.h>
#include <sys/stat.h>
//#include <sys/types.h>

#include <fcntl.h>

#include "BrowserHost.h"
#include "Codebendercc.h"
#include "DOM/Window.h"
#include "DOM/Document.h"
#include "global/config.h"
#include "JSAPIAuto.h"
#include "JSObject.h"
#include "src/3rdParty/boost/boost/thread/detail/thread.hpp"
#include "SimpleStreamHelper.h"
#include "variant_list.h"
#include "SimpleSerial.h"

#ifdef __APPLE__
#import <Security/Security.h>
#endif

#ifndef H_CodebenderccAPI
#define H_CodebenderccAPI

class CodebenderccAPI : public FB::JSAPIAuto {
public:
    ////////////////////////////////////////////////////////////////////////////
    ///
    ////////////////////////////////////////////////////////////////////////////
#define MSG_LEONARD_AUTORESET "Trying Arduino Leonardo auto-reset. If it does not reset automatically please reset the Arduino manually!"


    /**
     * Constructor for your JSAPI object.
     * You should register your methods, properties, and events
     * that should be accessible to Javascript from here.
     * 
     * @see FB::JSAPIAuto::registerMethod
     * @see FB::JSAPIAuto::registerProperty
     * @see FB::JSAPIAuto::registerEvent
     * @param plugin
     * @param host
     */
    CodebenderccAPI(const CodebenderccPtr& plugin, const FB::BrowserHostPtr& host) :
    m_plugin(plugin), m_host(host), io() {

        // Retrieve a reference to the DOM Window
        FB::DOM::WindowPtr window = m_host->getDOMWindow();

        //Register all JS callbacks
        registerMethod("probeUSB", make_method(this, &CodebenderccAPI::probeUSB));
        registerMethod("download", make_method(this, &CodebenderccAPI::download));
        registerMethod("flash", make_method(this, &CodebenderccAPI::flash));
        registerMethod("serialRead", make_method(this, &CodebenderccAPI::serialRead));
        registerMethod("disconnect", make_method(this, &CodebenderccAPI::disconnect));
        registerMethod("setCallback", make_method(this, &CodebenderccAPI::setCallback));
        registerMethod("serialWrite", make_method(this, &CodebenderccAPI::serialWrite));

        //Register all JS read-only properties
        registerProperty("version", make_property(this, &CodebenderccAPI::get_version));
        registerProperty("command", make_property(this, &CodebenderccAPI::getLastCommand));
        registerProperty("retVal", make_property(this, &CodebenderccAPI::getRetVal));

        std::string os = getPlugin().get()->getOS();
        path = getPlugin().get()->getFSPath();

        path = path.substr(0, path.find_last_of("/\\") + 1);

        std::string arch = "32";
#ifdef __x86_64
        arch = "64";
#endif

        //paths to files
        avrdude = path + os + ".avrdude";
        avrdudeConf = path + os + ".avrdude.conf";
        binFile = path + "file.bin";
        outfile = path + "out";

        if (os == "Windows") {
            //WINDOWS
            //libusb for windows
            libusb = path + "libusb0.dll";
            //.exe for windows
            avrdude = path + "avrdude.exe";
        } else if (os == "X11") {
            //LINUX
            avrdude = path + os + "." + arch + ".avrdude";
            avrdudeConf = path + os + "." + arch + ".avrdude.conf";
        } else {
            //MAC
            path = path + "../../";
            avrdude = path + os + ".avrdude";
            avrdudeConf = path + os + ".avrdude.conf";
            binFile = path + "file.bin";
            outfile = path + "out";
        }

        boost::thread t(boost::bind(&boost::asio::io_service::run, &io));

        serial = NULL;
        _retVal = 9999;
    }

    /**
     * Destructor.
     */
    virtual ~CodebenderccAPI() {
    };

    /**
     * Returns a reference to the Plugin Object.
     *   Gets a reference to the plugin that was passed in when the object
     *   was created.  If the plugin has already been released then this
     *   will throw a FB::script_error that will be translated into a
     *  javascript exception in the page.
     * @return a reference to the plugin.
     */
    CodebenderccPtr getPlugin();

    /**
     * Gets the plugin version.
     * @return the version string.
     */
    std::string get_version();


    /**
     * Used to Download Avrdude Objects.
     * @deprecated
     * This function is now available only for compatibility. No functionality here.
     * 
     * @return "deprecated".
     */
    FB::variant download();

    /**
     * Used to flash a binary file to a connected Arduino/device.
     * The flash operation is initiated in a @b new @b thread
     * @param device The port of the device as a string. @see validate_device
     * @param code A base64 encoded string of the binary file to be flashed to the device. 
     * @param maxsize The maximum size of a binary file that can be flashed to the specific Arduino/other Board.
     * @param protocol The protocol to be used for Avrdude.
     * @param speed The baudrate to be used with Avrdude.
     * @param mcu The mcu to be used with Avrdude.
     * @param cback A callback used to report the flash result.
     * @return 0 if the flash process is started. Anything else is an error value.
     */
    FB::variant flash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu, const FB::JSObjectPtr & cback);

	/**
     * When on Windows OS, finds all available usb ports.
     * @return a comma separated list of the detected devices.
     */
#if defined _WIN32 || _WIN64
	std::string CodebenderccAPI::QueryKey(HKEY hKey);
#endif
	
    /**
     * Checks for all available USB Arduino devices.
     * @return a comma separated list of the detected devices.
     */
    std::string probeUSB();
    /**
     * Returns the last avrdude's output.
     * @return the output recorded from avrdude.
     */
    FB::variant getFlashResult();
    /**
     * The last avrdude command executed.
     * @return the last avrdude command executed.
     */
    FB::variant getLastCommand();

    FB::variant getRetVal() {
        return _retVal;
    }

    /**
     * Sets a callback to notify the web page about changes.
     * A number of operational changes and exceptions are reported from here:
     * @param callback a javaScript callback function.
     * @return true if the callback is set, false in any error.
     */
    bool setCallback(const FB::JSObjectPtr &callback);
    /**
     * Opens a Serial Port and Reads input from it. 
     * The connection is maintained in a @b new @b thread.
     * 
     * @see serialRead
     * @param port The port of the device as a string. @see validate_device
     * @param baudrate The baudrate to use for the connection as a string.
     * @param callback A callback function to report all characters read from the Serial Port.
     * @return true if connection was attempted, false otherwise.
     */
    bool serialRead(const std::string &port, const std::string &baudrate, const FB::JSObjectPtr &callback);
    /**
     * Write String to the open serial port.
     * @param the string to write.
     */
    void serialWrite(const std::string &);
    /**
     * Disconnects from serial port.
     * @return 1 when disconnected. Any other value is an error value. 
     */
    FB::variant disconnect();


private:

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

    /**
     * Exec system command.
     * @param filename
     * @return 
     */
    std::string exec(const char * cmd);

    /**
     * Check if the file exists.
     * @param filename the file to check.
     * @return true if it exists.
     */
    bool fileExists(const std::string& filename);
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
    void saveToBin(unsigned char *, size_t);

    /**
     * Validate a number string.
     * @param input the string to validate.
     * @return true if the string contains a number, false else.
     */
    bool validate_number(const std::string &);
    /**
     * Validate a base64 file string.
     * @param input the code for the Arduino in base64 format.
     * @return true if the string is valid, false else.
     */
    bool validate_code(const std::string &);
    /**
     * Validate a string that contains digits and characters.
     * @param input the string to validate.
     * @return true if valid,false else.
     */
    bool validate_charnum(const std::string &);

    /**
     * Sends a notification to the default callback.
     */
    void notify(const std::string &message);

    /**
     * 
     * @param 
     * @param 
     * @param 
     * @param 
     * @param 
     * @param 
     * @param 
     */
    void doflash(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, const FB::JSObjectPtr &);

    /**
     * 
     * @param 
     * @param 
     * @param 
     */
    void serialReader(const std::string &, const unsigned int &, const FB::JSObjectPtr &);

    int runme(const std::string & cmd, const std::string & args);

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
    int _retVal;
    int mnum;

    FB::JSObjectPtr callback_;
    
    bool doclose;
    //    SimpleSerial * serial;
    boost::asio::io_service io;
    boost::asio::serial_port * serial;
    boost::array<char, 1 > buf;
    std::string path;
    int packets;

    void delay(int duration) {
#if defined _WIN32 || _WIN64
        Sleep(duration);
#else
        usleep(duration * 1000);
#endif
    }

#if defined _WIN32 || _WIN64

    std::wstring s2ws(const std::string& s) {
        int len;
        int slength = (int) s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }
#endif
};

#endif // H_CodebenderccAPI

