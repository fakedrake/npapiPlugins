/**********************************************************\

  Auto-generated CodebenderccAPI.h

\**********************************************************/

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/optional.hpp>
#include "JSAPIAuto.h"
#include "SimpleStreamHelper.h"
#include "BrowserHost.h"
#include "Codebendercc.h"

#ifndef H_CodebenderccAPI
#define H_CodebenderccAPI

#ifdef _WIN32
#define ISWIN
#endif
#ifdef _WIN64
#define ISWIN
#endif

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
		 
        // Read-only property
        registerProperty("version",
                make_property(this,
                &CodebenderccAPI::get_version));
        std::string os = getPlugin().get()->getOS();
        std::string path = getPlugin().get()->getFSPath();
		path = path.substr(0,path.find_last_of("/\\")+1);
        avrdude = path + "avrdude";
        avrdudeConf = path + "avrdude.conf";
        binFile = path + "file.bin";
        outfile = path + "out";
		
		libusb = path+ "libusb0.dll";
		if (os == "Windows") {
			avrdude = path+"avrdude.exe";			
		}

		//sitebase="http://codebender.cc/avrdude/";
		sitebase="http://exp.dev.codebender.cc/amaxilatis/Symfony/web/avrdude/";

    }

    ///////////////////////////////////////////////////////////////////////////////
    /// @fn CodebenderccAPI::~CodebenderccAPI()
    ///
    /// @brief  Destructor.  Remember that this object will not be released until
    ///         the browser is done with it; this will almost definitely be after
    ///         the plugin is released.
    ///////////////////////////////////////////////////////////////////////////////

    virtual ~CodebenderccAPI() {
    };

    CodebenderccPtr getPlugin();

    // Read-only property ${PROPERTY.ident}
    std::string get_version();

    FB::variant download();
    void getURL(const std::string& url, const std::string& destination);
    FB::variant flash(const std::string& device, const std::string& code, const std::string& maxsize, const std::string& protocol, const std::string& speed, const std::string& mcu);
    FB::variant probeUSB();
    FB::variant getFlashResult();



private:
#ifdef ISWIN
	FB::variant probeUSB_win();
#endif
    bool fileExists(const std::string& filename);
    void getURLCallback(bool success,
            const FB::HeaderMap& headers, const boost::shared_array<uint8_t>& data, const size_t size, const std::string& destination);
    std::string exec(const char * cmd);
    size_t base64_decode(const char *source, unsigned char *target, size_t targetlen);
    int _base64_char_value(char base64char);
    int _base64_decode_triple(char quadruple[4], unsigned char *result);
    void saveToBin(unsigned char * data, size_t size);

    
    CodebenderccWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

    std::string m_testString;

    std::string avrdude, avrdudeConf, binFile, outfile;
	std::string libusb;
	std::string sitebase;
};

#endif // H_CodebenderccAPI

