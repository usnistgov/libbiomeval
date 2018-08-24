/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#include <be_io_utility.h>
#include <be_memory_autoarray.h>

#include <gtest/gtest.h>

namespace BE = BiometricEvaluation;

/** @return true if a and b are different, false otherwise. */
bool
different(
    const BiometricEvaluation::Memory::uint8Array &a,
    const BiometricEvaluation::Memory::uint8Array &b)
{
	if (a.size() != b.size())
		return (true);
	
	for (BE::Memory::uint8Array::size_type i = 0; i < a.size(); i++)
		if (a[i] != b[i])
			return (true);
	return (false);
}


TEST(IOUtility, FileIO)
{
	const std::string filename = "test_be_io_utility.cpp";
	/* Read file */
	BE::Memory::uint8Array originalFile;
	EXPECT_NO_THROW(originalFile = BE::IO::Utility::readFile(filename));

	/* Write file */
	const std::string tempFileName = "temp_file";
	if (BE::IO::Utility::fileExists(tempFileName))
		ASSERT_EQ(unlink(tempFileName.c_str()), 0);
	EXPECT_NO_THROW(BE::IO::Utility::writeFile(originalFile, tempFileName));
	/* Check written file */
	BE::Memory::uint8Array readFile;
	EXPECT_NO_THROW(readFile = BE::IO::Utility::readFile(tempFileName));
	ASSERT_FALSE(different(originalFile, readFile));
	ASSERT_NO_THROW(ASSERT_EQ(BE::IO::Utility::getFileSize(tempFileName),
	    originalFile.size()));

	/* Overwrite existing file without truncation flag */
	EXPECT_THROW(BE::IO::Utility::writeFile(originalFile, tempFileName),
	    BE::Error::ObjectExists);
	
	/* Overwrite existing file with truncation flag */
	EXPECT_NO_THROW(BE::IO::Utility::writeFile(originalFile, tempFileName,
	    std::ios_base::trunc));
	/* Check written file */
	EXPECT_NO_THROW(readFile = BE::IO::Utility::readFile(tempFileName));
	ASSERT_FALSE(different(originalFile, readFile));

	EXPECT_EQ(0, unlink(tempFileName.c_str()));
}

TEST(IOUtility, SetAside)
{
	const std::string filename = "test_be_io_utility.cpp";
	const std::string tempFileName = "temp_file";
	BE::Memory::uint8Array originalFile, readFile;
	ASSERT_NO_THROW(originalFile = BE::IO::Utility::readFile(filename));
	ASSERT_NO_THROW(BE::IO::Utility::writeFile(originalFile, tempFileName));

	/* Test the set aside of a file */
	const uint8_t maxIterations = 5;
	for (uint8_t i = 1; i <= maxIterations; i++) {
		EXPECT_NO_THROW(BE::IO::Utility::setAsideName(tempFileName));
		EXPECT_TRUE(BE::IO::Utility::fileExists(tempFileName + '.' + 
		    std::to_string(i)));

		/* Ensure original file is gone */
		EXPECT_NO_THROW(BE::IO::Utility::writeFile(originalFile,
		    tempFileName));

		/* Ensure file wasn't modified */
		EXPECT_NO_THROW(readFile = BE::IO::Utility::readFile(
		    tempFileName + '.' + std::to_string(i)));
		EXPECT_FALSE(different(originalFile, readFile));
	}
	EXPECT_EQ(0, unlink(tempFileName.c_str()));
	for (uint8_t i = 1; i <= maxIterations; i++)
		EXPECT_EQ(0,
		    unlink((tempFileName + '.' + std::to_string(i)).c_str()));

	/* Test the set aside of a directory */
	const std::string tempDirName = "temp_dir";
	ASSERT_EQ(0, mkdir(tempDirName.c_str(), 0777));
	for (uint8_t i = 1; i <= maxIterations; i++) {
		EXPECT_NO_THROW(BE::IO::Utility::setAsideName(tempDirName));
		EXPECT_TRUE(BE::IO::Utility::fileExists(tempDirName + '.' + 
		    std::to_string(i)));

		EXPECT_EQ(0, mkdir(tempDirName.c_str(), 0777));
	}
	EXPECT_EQ(0, rmdir(tempDirName.c_str()));
	for (uint8_t i = 1; i <= maxIterations; i++)
		EXPECT_EQ(0,
		    rmdir((tempDirName + '.' + std::to_string(i)).c_str()));
}

TEST(IOUtility, Trees)
{
	const std::string tempDirName = "temp_dir";
	ASSERT_FALSE(BE::IO::Utility::fileExists(tempDirName));

	/* Create a directory path */
	std::string firstLvl1 = tempDirName + "/temp";
	std::string endLvls = "foo/bar";
	std::string tree1 = firstLvl1 + "/" + endLvls;
	EXPECT_NO_THROW(BE::IO::Utility::makePath(tree1, 0777));
	EXPECT_TRUE(BE::IO::Utility::fileExists(tree1));

	/*
 	 * Copy the contents of a directory from the top level.
 	 * create a file at the bottom to exercise file copying as well.
 	 */
	auto textFile = BE::IO::Utility::readFile("test_be_io_utility.cpp");
	const std::string tempFileName = "temp_file";
	BE::IO::Utility::writeFile(textFile, tree1 + "/" + tempFileName);

	std::string firstLvl2 = tempDirName + "/temp2";
	std::string tree2 = firstLvl2 + "/" + endLvls;
	EXPECT_NO_THROW(BE::IO::Utility::copyDirectoryContents(firstLvl1,
	    firstLvl2, true));

	EXPECT_TRUE(BE::IO::Utility::fileExists(tree2 + "/" + tempFileName));
	EXPECT_FALSE(BE::IO::Utility::fileExists(firstLvl1));

	/* Remove a directory tree */
	EXPECT_NO_THROW(BE::IO::Utility::removeDirectory(tempDirName));
	EXPECT_FALSE(BE::IO::Utility::fileExists(tempDirName));
}

TEST(IOUtility, TemporaryFiles)
{
	std::string testTempFile;
	EXPECT_NO_THROW(testTempFile = BE::IO::Utility::createTemporaryFile(
	    "test"));
	EXPECT_EQ(0, unlink(testTempFile.c_str()));
	
	FILE *tempFp = nullptr;
	EXPECT_NO_THROW(tempFp = BE::IO::Utility::createTemporaryFile(
	    testTempFile, "test"));

	std::string testContents = "This is a test entry for the temp file";
	ASSERT_EQ(std::fwrite(testContents.c_str(), 1, testContents.size(),
	    tempFp), testContents.size());
	EXPECT_EQ(0, fclose(tempFp));

	auto testContentsRead = BE::IO::Utility::readFile(testTempFile);
	EXPECT_EQ(testContentsRead.size(), testContents.size());
	for (BE::Memory::uint8Array::size_type i = 0;
	    i < testContents.size(); i++)
		ASSERT_EQ(testContentsRead[i], testContents[i]);
	EXPECT_EQ(0, unlink(testTempFile.c_str()));
}

TEST(IOUtility, PathIsDirectory)
{
	EXPECT_NO_THROW(EXPECT_FALSE(BE::IO::Utility::pathIsDirectory("")));
	EXPECT_NO_THROW(EXPECT_TRUE(BE::IO::Utility::pathIsDirectory(".")));
	EXPECT_NO_THROW(EXPECT_FALSE(BE::IO::Utility::pathIsDirectory(
	    "test_be_io_utility.cpp")));
	EXPECT_NO_THROW(EXPECT_FALSE(BE::IO::Utility::pathIsDirectory(
	    "DoesNotExist")));
}

#if 0
TEST(IOUtility, LargeFiles)
{
	/* Write 4GB + 1 file */
	const uint64_t fourPlusOne = 0xFFFFFFFFull + 1;
	const std::string fileName = "temp_file";
	BE::Memory::uint8Array buf(fourPlusOne);
	EXPECT_NO_THROW(BE::IO::Utility::writeFile(buf, fileName));

	EXPECT_TRUE(BE::IO::Utility::fileExists(fileName));
	EXPECT_TRUE(BE::IO::Utility::isReadable(fileName));
	EXPECT_TRUE(BE::IO::Utility::isWritable(fileName));
	EXPECT_EQ(fourPlusOne, BE::IO::Utility::getFileSize(fileName));

	EXPECT_EQ(0, unlink(fileName.c_str()));
}
#endif

TEST(IOUtility, Permissions)
{
	/* Invalid path */
	EXPECT_FALSE(BE::IO::Utility::isReadable(""));
	EXPECT_FALSE(BE::IO::Utility::isWritable(""));

	/* CWD */
	EXPECT_TRUE(BE::IO::Utility::isReadable("."));
	EXPECT_TRUE(BE::IO::Utility::isWritable("."));

	EXPECT_FALSE(BE::IO::Utility::isReadable("BadFile"));
	EXPECT_FALSE(BE::IO::Utility::isWritable("BadFile"));

	EXPECT_TRUE(BE::IO::Utility::isReadable("test_be_io_utility"));
	EXPECT_TRUE(BE::IO::Utility::isWritable("test_be_io_utility"));

	const std::string tempFileName = "temp_file";
	BE::Memory::uint8Array empty;
	BE::IO::Utility::writeFile(empty, tempFileName);

	EXPECT_EQ(0, chmod(tempFileName.c_str(), 0400));
	EXPECT_TRUE(BE::IO::Utility::isReadable(tempFileName));
	EXPECT_FALSE(BE::IO::Utility::isWritable(tempFileName));

	EXPECT_EQ(0, chmod(tempFileName.c_str(), 0222));
	EXPECT_FALSE(BE::IO::Utility::isReadable(tempFileName));
	EXPECT_TRUE(BE::IO::Utility::isWritable(tempFileName));

	EXPECT_EQ(0, chmod(tempFileName.c_str(), 0007));
	EXPECT_FALSE(BE::IO::Utility::isReadable(tempFileName));
	EXPECT_FALSE(BE::IO::Utility::isWritable(tempFileName));

	EXPECT_EQ(0, unlink(tempFileName.c_str()));
}

