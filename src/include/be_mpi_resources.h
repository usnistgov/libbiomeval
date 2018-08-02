/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_RESOURCES_H
#define _BE_MPI_RESOURCES_H

#include <memory>
#include <string>
#include <vector>

namespace BiometricEvaluation {
	namespace MPI {
		/**
		 * A class to represent a set of resources needed by an
		 * MPI program. The resources are based on a properties
		 * file as well as some dynamic information, such as MPI
		 * rank and process ID.
 		 */
		class Resources {
		public:
			/**
			 * @brief
			 * The property string "Workers Per Node"; required.
			 * @details
			 * This value shall be either an integer or one of
			 * the strings "NUMCPUS", "NUMCORES", "NUMSOCKETS".
			 */
			static const std::string WORKERSPERNODEPROPERTY;

			/**
			 * @brief
			 * The "Workers Per Node" setting "NUMCPUS".
			 * @details
			 * This setting indicates the MPI Framework is to
			 * create one worker for each logical CPU.
			 */
			static const std::string NUMCPUS;

			/**
			 * @brief
			 * The "Workers Per Node" setting "NUMCORES".
			 * @details
			 * This setting indicates the MPI Framework is to
			 * create one worker for each physical CPU core.
			 */
			static const std::string NUMCORES;

			/**
			 * @brief
			 * The "Workers Per Node" setting "NUMSOCKETS".
			 * @details
			 * This setting indicates the MPI Framework is to
			 * create one worker for each physical CPU socket.
			 */
			static const std::string NUMSOCKETS;

			/**
			 * @brief
			 * The property string "Logsheet URL"; optional.
			 */
			static const std::string LOGSHEETURLPROPERTY;

			/**
			 * @brief
			 * Obtain the list of required properties.
			 * @return
			 * A set of required property strings.
			 */
			static std::vector<std::string>
			    getRequiredProperties();

			/**
			 * @brief
			 * Obtain the list of optional properties.
			 * @return
			 * A set of optional property strings.
			 */
			static std::vector<std::string>
			    getOptionalProperties();

			/**
			 * @brief
			 * Constructor taking the name of the properties
			 * file describing the resources.
			 * @param[in] propertiesFileName
			 * The name of the file containing the Properties.
			 * @throw Error::FileError
			 * The resources file could not be read.
			 * @throw Error::ObjectDoesNotExist
			 * A required property does not exist.
			 * @throw Error::Exception
			 * Some other error occurred.
			 */
			Resources(const std::string &propertiesFileName);

			/**
			 * @brief
			 * Obtain the name of the file used to construct this
			 * object.
			 * @return
			 * The name of the properties file.
			 */
			std::string getPropertiesFileName() const;

			/**
			 * @brief
			 * Obtain the Uniform Resource Locator for the
			 * IO:Logsheet object.
			 * @details
			 * This string my be empty, indicating that there
			 * is no Logsheet URL in the Properties file.
			 * @return
			 * The Logsheet URL.
			 */
			std::string getLogsheetURL() const;

			~Resources();

			int getRank() const;
			int getNumTasks() const;
			int getWorkersPerNode() const;

		private:
			std::string _propertiesFileName;
			int _rank;
			int _numTasks;
			int _workersPerNode;
			std::string _logsheetURL;
		};
	}
}

#endif /* _BE_MPI_RESOURCES_H */

