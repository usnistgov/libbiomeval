/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cmath>
#include <cstdlib>
#include <limits>

#include <be_io_properties.h>
#include <be_io_utility.h>
#include <be_error_exception.h>

#include <sys/stat.h>

#include <gtest/gtest.h>

#ifdef PROPERTIESFILE
const std::string propsname = "propsfile.props";
#include <be_io_propertiesfile.h>
#endif

namespace BE = BiometricEvaluation;


TEST(Properties, ReadOnly)
{
#ifdef PROPERTIESFILE
	/* Can't open a new properties file read-only */
	EXPECT_THROW(std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsname, BE::IO::Mode::ReadOnly)),
	    BE::Error::StrategyError);
#else
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::Properties(BE::IO::Mode::ReadOnly));

	const std::string key = "key";
	EXPECT_THROW(props->setProperty(key, "0"), BE::Error::StrategyError);
	EXPECT_THROW(props->setPropertyFromDouble(key, 0),
	    BE::Error::StrategyError);
	EXPECT_THROW(props->setPropertyFromInteger(key, 0),
	    BE::Error::StrategyError);

	EXPECT_THROW(props->removeProperty(key), BE::Error::StrategyError);
#endif
}

TEST(Properties, InsertAndReadString)
{
#ifdef PROPERTIESFILE
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsname));
#else
	std::unique_ptr<BE::IO::Properties> props(new BE::IO::Properties());
#endif

	std::string property = "   string Prop   ";
	std::string value = "John   Smith    ";
	EXPECT_NO_THROW(props->setProperty(property, value));

	/* Retrieve via unstripped key */
	EXPECT_NO_THROW(EXPECT_EQ("John   Smith",
	    props->getProperty(property)));
	/* Retrieve via stripped key */
	EXPECT_NO_THROW(EXPECT_EQ("John   Smith",
	    props->getProperty("string Prop")));
	/* Retrieve via invalid key */
	EXPECT_THROW(props->getProperty("string prop"),
	    BE::Error::ObjectDoesNotExist);
	/* Retrieve as invalid datatypes */
	EXPECT_THROW(props->getPropertyAsDouble(property),
	    BE::Error::ConversionError);
	EXPECT_THROW(props->getPropertyAsInteger(property),
	    BE::Error::ConversionError);

#ifdef PROPERTIESFILE
	props.reset(nullptr);
	ASSERT_EQ(BE::IO::Utility::getFileSize(propsname), 27);
	ASSERT_EQ(::unlink(propsname.c_str()), 0);
	ASSERT_FALSE(BE::IO::Utility::fileExists(propsname));
#endif
}

TEST(Properties, InsertAndReadInteger)
{
#ifdef PROPERTIESFILE
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsname));
#else
	std::unique_ptr<BE::IO::Properties> props(new BE::IO::Properties());
#endif

	std::string property = "   int Prop   ";
	uint64_t value = 0xFFFFFFFF9ull;
	EXPECT_NO_THROW(props->setPropertyFromInteger(property, value));

	/* Retrieve via unstripped key */
	EXPECT_NO_THROW(EXPECT_EQ(value,
	    props->getPropertyAsInteger(property)));
	/* Retrieve via stripped key */
	EXPECT_NO_THROW(EXPECT_EQ(value,
	    props->getPropertyAsInteger("int Prop")));
	/* Retrieve via invalid key */
	EXPECT_THROW(props->getPropertyAsInteger("int prop"),
	    BE::Error::ObjectDoesNotExist);
	/* Retrieve as other valid datatypes */
	EXPECT_NO_THROW(EXPECT_EQ(value, 
	    props->getPropertyAsDouble(property)));
	EXPECT_NO_THROW(EXPECT_EQ("68719476729",
	    props->getProperty(property)));

	/* Signed value */
	std::string negativeProperty = "negative";
	int32_t negativeValue = -233453;
	EXPECT_NO_THROW(props->setPropertyFromInteger(negativeProperty,
	    negativeValue));
	EXPECT_NO_THROW(EXPECT_EQ(negativeValue,
	    props->getPropertyAsInteger(negativeProperty)));
	EXPECT_NO_THROW(EXPECT_EQ("-233453",
	    props->getProperty(negativeProperty)));

	EXPECT_NO_THROW(props->setProperty(property, "10 11"));
	EXPECT_THROW(props->getPropertyAsInteger(property),
	    BE::Error::ConversionError);

#ifdef PROPERTIESFILE
	props.reset(nullptr);
	ASSERT_EQ(BE::IO::Utility::getFileSize(propsname), 36);
	ASSERT_EQ(::unlink(propsname.c_str()), 0);
	ASSERT_FALSE(BE::IO::Utility::fileExists(propsname));
#endif
}

TEST(Properties, InsertAndReadDouble)
{
#ifdef PROPERTIESFILE
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsname));
#else
	std::unique_ptr<BE::IO::Properties> props(new BE::IO::Properties());
#endif

	std::string property = "   double Prop   ";
	double value = 234876.232553;
	EXPECT_NO_THROW(props->setPropertyFromDouble(property, value));

	/* Retrieve via unstripped key */
	EXPECT_NO_THROW(EXPECT_EQ(value,
	    props->getPropertyAsDouble(property)));
	/* Retrieve via stripped key */
	EXPECT_NO_THROW(EXPECT_EQ(value,
	    props->getPropertyAsDouble("double Prop")));
	/* Retrieve via invalid key */
	EXPECT_THROW(props->getPropertyAsDouble("double prop"),
	    BE::Error::ObjectDoesNotExist);
	/* Can't retrieve as integer because . is not a digit */
	EXPECT_THROW(props->getPropertyAsInteger(property),
	    BE::Error::ConversionError);
	/* Can receive as a string */
	EXPECT_NO_THROW(EXPECT_EQ("234876.232553",
	    props->getProperty(property)));
	/* Fail because of space */
    	EXPECT_NO_THROW(props->setProperty(property, "10 11"));
	EXPECT_THROW(props->getPropertyAsDouble(property),
	    BE::Error::ConversionError);

#ifdef PROPERTIESFILE
	props.reset(nullptr);
	ASSERT_EQ(BE::IO::Utility::getFileSize(propsname), 20);
	ASSERT_EQ(::unlink(propsname.c_str()), 0);
	ASSERT_FALSE(BE::IO::Utility::fileExists(propsname));
#endif
}

TEST(Properties, StringsAsNumbers)
{
#ifdef PROPERTIESFILE
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsname));
#else
	std::unique_ptr<BE::IO::Properties> props(new BE::IO::Properties());
#endif

	std::string property = "numberString";
	std::string value = "0x1234";
	EXPECT_NO_THROW(props->setProperty(property, value));
	EXPECT_NO_THROW(EXPECT_EQ(value, props->getProperty(property)));
	EXPECT_NO_THROW(EXPECT_EQ(0x1234,
	    props->getPropertyAsInteger(property)));
	EXPECT_NO_THROW(EXPECT_EQ(0x1234,
	    props->getPropertyAsDouble(property)));

	value = "0X4567";
	EXPECT_NO_THROW(props->setProperty(property, value));
	EXPECT_NO_THROW(EXPECT_EQ(value, props->getProperty(property)));
	EXPECT_NO_THROW(EXPECT_EQ(0x4567,
	    props->getPropertyAsInteger(property)));
	EXPECT_NO_THROW(EXPECT_EQ(0x4567,
	    props->getPropertyAsDouble(property)));

	value = "0xDEFG";
	EXPECT_NO_THROW(props->setProperty(property, value));
	EXPECT_NO_THROW(EXPECT_EQ(value, props->getProperty(property)));
	EXPECT_THROW(props->getPropertyAsInteger(property),
	    BE::Error::ConversionError);
	EXPECT_THROW(props->getPropertyAsDouble(property),
	    BE::Error::ConversionError);

	/* Out of range */
	value = "9999999999999999999999999999999999999999999999999999999999";
	EXPECT_NO_THROW(props->setProperty(property, value));
	EXPECT_NO_THROW(EXPECT_EQ(value, props->getProperty(property)));
	EXPECT_THROW(props->getPropertyAsInteger(property),
	    BE::Error::ConversionError);
	/* Double actually *can* handle this... */
	EXPECT_NO_THROW(EXPECT_EQ(props->getPropertyAsDouble(property),
	    1e58 - 1));
	/* ...but not this */
	value = "1e8923749823784";
	EXPECT_NO_THROW(props->setProperty(property, value));
	EXPECT_THROW(props->getPropertyAsDouble(property),
	    BE::Error::ConversionError);

#ifdef PROPERTIESFILE
	props.reset(nullptr);
	ASSERT_EQ(BE::IO::Utility::getFileSize(propsname), 31);
	ASSERT_EQ(::unlink(propsname.c_str()), 0);
	ASSERT_FALSE(BE::IO::Utility::fileExists(propsname));
#endif
}

TEST(Properties, SpecialDoubleCases)
{
#ifdef PROPERTIESFILE
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsname));
#else
	std::unique_ptr<BE::IO::Properties> props(new BE::IO::Properties());
#endif
	std::string property = "property";

	/* Exponents */
	EXPECT_NO_THROW(props->setProperty(property, "-2e-3"));
	EXPECT_NO_THROW(EXPECT_EQ(props->getPropertyAsDouble(property),
	    -0.002));
	EXPECT_NO_THROW(props->setProperty(property, "+6E+3"));
	EXPECT_NO_THROW(EXPECT_EQ(props->getPropertyAsDouble(property),
	    6000));
	
	/* Hexadecimal fractions and exponents */	
	EXPECT_NO_THROW(props->setProperty(property, "+0xFF.1p-2"));
	EXPECT_NO_THROW(EXPECT_EQ(props->getPropertyAsDouble(property),
	    63.765625));

	/* Infinity */
	EXPECT_NO_THROW(props->setProperty(property, "+INF"));
	EXPECT_NO_THROW(EXPECT_TRUE(std::isinf(
	    props->getPropertyAsDouble(property))));
	EXPECT_NO_THROW(props->setProperty(property, "INFINITY"));
	EXPECT_NO_THROW(EXPECT_TRUE(std::isinf(
	    props->getPropertyAsDouble(property))));
	EXPECT_NO_THROW(props->setProperty(property, "INFI"));
	EXPECT_THROW(props->getPropertyAsDouble(property),
	    BE::Error::ConversionError);
	
	/* Not a number */
	EXPECT_NO_THROW(props->setProperty(property, "NAN"));
	EXPECT_NO_THROW(EXPECT_TRUE(std::isnan(
	    props->getPropertyAsDouble(property))));

	/* Numbers with spaces are okay, but we say no */
	EXPECT_NO_THROW(props->setProperty(property, "10 11"));
	EXPECT_THROW(props->getPropertyAsDouble(property),
	    BE::Error::ConversionError);

#ifdef PROPERTIESFILE
	props.reset(nullptr);
	ASSERT_EQ(BE::IO::Utility::getFileSize(propsname), 17);
	ASSERT_EQ(::unlink(propsname.c_str()), 0);
	ASSERT_FALSE(BE::IO::Utility::fileExists(propsname));
#endif
}

TEST(Properties, InvalidProperties)
{
#ifdef PROPERTIESFILE
	std::unique_ptr<BE::IO::Properties> props(
	    new BE::IO::PropertiesFile(propsname));
#else
	std::unique_ptr<BE::IO::Properties> props(new BE::IO::Properties());
#endif

	std::string property = "BadProperty";
	EXPECT_THROW(props->getProperty(property),
	    BE::Error::ObjectDoesNotExist);
	EXPECT_THROW(props->getPropertyAsInteger(property),
	    BE::Error::ObjectDoesNotExist);
	EXPECT_THROW(props->getPropertyAsDouble(property),
	    BE::Error::ObjectDoesNotExist);

#ifdef PROPERTIESFILE
	props.reset(nullptr);
	ASSERT_EQ(BE::IO::Utility::getFileSize(propsname), 0);
	ASSERT_EQ(::unlink(propsname.c_str()), 0);
	ASSERT_FALSE(BE::IO::Utility::fileExists(propsname));
#endif
}

#ifdef PROPERTIESFILE
TEST(PropertiesFile, ChangeName)
{
	std::unique_ptr<BE::IO::PropertiesFile> props(
	    new BE::IO::PropertiesFile(propsname));

	/* Add some contents to the file and sync */
	std::string key = "key";
	uint8_t value = 24;
	EXPECT_NO_THROW(props->setPropertyFromInteger(key, value));
	EXPECT_NO_THROW(props->sync());

	/* Load the contents of the file */
	EXPECT_TRUE(BE::IO::Utility::fileExists(propsname));
	BE::Memory::uint8Array beforeFileContents = BE::IO::Utility::readFile(
	    propsname);
	EXPECT_EQ(BE::IO::Utility::getFileSize(propsname), 9);

	/* Change the name */
	std::string newPropsName = propsname + ".tmp";
	EXPECT_NO_THROW(props->changeName(newPropsName));

	EXPECT_FALSE(BE::IO::Utility::fileExists(propsname));
	BE::Memory::uint8Array afterFileContents = BE::IO::Utility::readFile(
	    propsname + ".tmp");
	EXPECT_EQ(beforeFileContents.size(), afterFileContents.size());
	for (uint64_t i = 0; i < afterFileContents.size(); i++)
		EXPECT_EQ(afterFileContents[i], beforeFileContents[i]);
	EXPECT_EQ(props->getPropertyAsInteger(key), value);

	props.reset(nullptr);
	ASSERT_EQ(::unlink(newPropsName.c_str()), 0);
	ASSERT_FALSE(BE::IO::Utility::fileExists(newPropsName));
}
#endif

