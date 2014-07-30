/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_IO_LOGSHEET_H__
#define __BE_IO_LOGSHEET_H__

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

namespace BiometricEvaluation
{
	namespace IO
	{
		/**
		 * @brief
		 * A class to represent a logging mechanism.
		 *
		 * @details
		 * A Logsheet is a string stream, so applications can write into
		 * the stream as a staging area using the << operator, then
		 * start a new entry by calling newEntry(). Entries in the log
		 * are prefixed with an entry number, which is incremented
		 * when the entry is written (either by directly calling
		 * write(), or calling newEntry()).
		 *
		 * How the log data is stored is implemented by subclasses
		 * of Logsheet.
		 *
		 * @note
	 	 * By default, the entries in the Logsheet may not be
		 * immediately written to the backing store, depending on the
		 * buffering behavior of the operating system. Applications
		 * can force a write by invoking sync(), or force a write at
		 * every new log entry by invoking setAutoSync(true).
		 * @note
		 * Entries created by applications may be composed of more than
		 * one line (each separated by the newline character). The text
		 * at the beginning of a line should not "look like" an entry
		 * number:
		 * \n
		 *  Edddd
		 * \n
		 * i.e. the entry delimiter followed by some digits. Logsheet
		 * won't check for that condition, but any existing Logsheet
		 * that is re-opened for append may have an incorrect starting
		 * entry number.
		 */
		class Logsheet : public std::ostringstream
		{
		public:
			enum class Kind {
				/** No backing store log sheet */
				Null,
				/** File-based log sheet */
				File,
				/** Syslog daemon backing store */
				Syslog
			};

			/** Delimiter for a comment line in the log sheet. */
			static const char CommentDelimiter = '#';

			/** Delimiter for an entry line in the log sheet. */
			static const char EntryDelimiter = 'E';

			/** Delimiter for an debug line in the log sheet. */
			static const char DebugDelimiter = 'D';

			/** The tag for the description string. */
			static const std::string DescriptionTag;

			/**
			 * The URL scheme to be used for FileLogsheet
			 * URL strings.
			 */
			static const std::string FILEURLSCHEME;

			/**
			 * The URL scheme to be used for SysLogsheet
			 * URL strings.
			 */
			static const std::string SYSLOGURLSCHEME;

			/**
			 * @brief
			 * Map the URL scheme, taken from a string containing
			 * the entire URL, into a Logsheet type.
			 *
			 * @param[in] url
			 *	The unform resource locator of the Logsheet.
			 * @return
			 *	The type of Logsheet represented by the URL.
			 * @throw Error::ParameterError
			 *	The URL scheme is missing or invalid.
			 */
			static Logsheet::Kind getTypeFromURL(
			    const std::string &url);

			/**
			 * @brief
			 * Create a Logsheet that has no backing store.
			 * A log entry is maintained, but cannot be
			 * permanently stored. This is the Null Logsheet.
			 */
			Logsheet();

			/**
			 * @brief
			 * Helper function to determine whether a string is
			 * a valid log entry.
			 *
			 * @param[in] line
			 * The string potentially containing a log entry.
			 * @return
			 * true if the string is a log entry, false otherwise.
			 */
			static bool
			lineIsEntry(const std::string &line);

			/**
			 * @brief
			 * Helper function to determine whether a string is
			 * a valid comment log entry.
			 *
			 * @param[in] line
			 * The string potentially containing a comment entry.
			 * @return
			 * true if the string is a comment entry, false
			 * otherwise.
			 */
			static bool
			lineIsComment(const std::string &line);

			/**
			 * @brief
			 * Helper function to determine whether a string is
			 * a valid debug log entry.
			 *
			 * @param[in] line
			 * The string potentially containing a debug entry.
			 * @return
			 * true if the string is a debug entry, false
			 * otherwise.
			 */
			static bool
			lineIsDebug(const std::string &line);

			/** Destructor */
			virtual ~Logsheet();

			/**
			 * @brief
			 * Start a new entry, causing the existing entry
			 * to be closed and written.
			 * @details
			 * Applications do not have to call this method
			 * for the first entry, however, as the stream
			 * is ready for writing upon construction.
			 *
			 * @throw Error::StrategyError
			 * An error occurred when using the underlying
			 * backing store.
			 */
			void
			newEntry();

			/**
			 * @brief
			 * Obtain the contents of the current entry currently
			 * under construction.
			 *
			 * @return
			 * The text of the current entry.
			 */
			std::string
			getCurrentEntry() const;

			/** Reset the current entry buffer to the beginning. */
			void
			resetCurrentEntry();

			/**
			 * @brief
			 * Obtain the current entry number.
			 *
			 * @return
			 * The current entry number.
			 */
			uint32_t
			getCurrentEntryNumber() const;

			/**
			 * @brief
			 * Write a string as an entry to the backing store.
			 * @details
			 * This does not affect the current log entry buffer,
			 * but does increment the entry number.
			 *
			 * @param[in] entry
			 * The text of the log entry.
			 *
			 * @throw Error::StrategyError
			 * An error occurred when using the underlying
			 * backing store.
			 */
			virtual void
			write(
			    const std::string &entry);

			/**
			 * @brief
			 * Write a string as a comment to the backing store.
			 * @details
			 * This does not affect the current log entry buffer,
			 * and does not increment the entry number. A comment
			 * line is prefixed with CommentDelimiter followed by
			 * a space by this method.
			 *
			 * @param[in] entry
			 * The text of the comment.
			 *
			 * @throw Error::StrategyError
			 * An error occurred when using the underlying
			 * backing store.
			 */
			virtual void
			writeComment(
			    const std::string &entry);

			/**
			 * @brief
			 * Write a string as a debug entry to the backing store.
			 * @details
			 * This does not affect the current log entry buffer,
			 * and does not increment the entry number. A debug
			 * line is prefixed with DebugDelimiter followed by
			 * a space.
			 *
			 * @param[in] entry
			 *	The text of the debug message.
			 *
			 * @throw Error::StrategyError
			 *	An error occurred when logging.
			 */
			virtual void
			writeDebug(const std::string &entry);

			/**
			 * @brief
			 * Enable or disable the commitment of normal entries
			 * to the backing log storage.
			 * @details
			 * When entry commitment is disabled, the entry number
			 * is not incremented. Entries may be streamed into
			 * the object, and new entries created.
			 *
			 * @param[in] state
			 *	True if normal entries are to be committed,
			 *	false if not.
			 */
			void
			setCommit(const bool state);

			/**
			 * @brief
			 * Get the current entry commit state.
			 *
			 * @return
			 *	true if normal entries are to be committed,
			 *	false if not.
			 */
			bool
			getCommit() const;

			/**
			 * @brief
			 * Enable or disable the commitment of debug entries
			 * to the backing log storage.
			 * @details
			 * When debug entry commitment is disabled, calls to
			 * writeDebug may still be made, but those entries do
			 * not appear in the log backing store.
			 *
			 * @param[in] state
			 *	true if debug entries are to be committed,
			 *	false if not.
			 */
			void
			setDebugCommit(const bool state);

			/**
			 * @brief
			 * Get the current debug entry commit state.
			 * 
			 * @return
			 *	true if debug entries are committed to the
			 *	backing store, false otherwise.
			*/
			bool
			getDebugCommit() const;

			/**
			 * @brief
			 * Enable or disable the commitment of comment entries
			 * to the backing log storage.
			 * @details
			 * When comment entry commitment is disabled, calls to
			 * writeComment may still be made, but those entries do
			 * not appear in the log backing store.
			 *
			 * @param[in] state
			 *	true if comment entries are to be committed,
			 *	false if not.
			 */
			void
			setCommentCommit(const bool state);

			/**
			 * @brief
			 * Get the current comment entry commit state.
			 * 
			 * @return
			 *	true if comment entries are committed to the
			 *	backing store, false otherwise.
			*/
			bool
			getCommentCommit() const;

			/**
			 * @brief
			 * Synchronize any buffered data to the underlying
			 * backing store.
			 * @details
			 * This syncing is dependent on the behavior of the
			 * underlying storage mechanism.
			 *
			 * @throw Error::StrategyError
			 * An error occurred when using the underlying
			 * backing store.
			*/
			virtual void
			sync();

			/**
			 * Turn on/off auto-sync of the data. Applications may
			 * gain performance by turning off auto-sync, or gain
			 * reliability by turning it on.
			 * 
			 * @param state
			 * When true, the data is sync'd whenever
			 * newEntry() is or write() is called. When 
			 * false, sync() must be called to force a write.
			*/
			void
			setAutoSync(bool state);

			/**
			 * Return the current auto-sync state.
			 * 
			 * @return
			 * true if auto-sync is on, false otherwise.
			*/
			bool
			getAutoSync() const;

			/**
			 * @brief
			 * Trim delimiters from Logsheet entries.
			 * @details
			 * Works for comments and numbered entries.
			 *
			 * @param[in] entry
			 * The entry to trim.
			 *
			 * @return
			 * Delimiter-less entry.
			 */
			static std::string
			trim(
			    const std::string &entry);
			    
		protected:
			/**
			 * @brief
			 * Increment the current entry number.
			 */
			void
			incrementEntryNumber();

			/**
			 * @brief
			 * Obtain the current entry 'tag', in 'Edddd' format.
			 *
			 * @return
			 * The text of the current entry tag.
			 */
			std::string
			getCurrentEntryNumberAsString() const;

		private:
			/** Number of the current entry */
			uint32_t _entryNumber;

			/** Whether or not to sync() on write() */
			bool _autoSync;

			/** Whether normal entries are committed */
			bool _commit;

			/** Whether debug entries are committed */
			bool _debugCommit;

			/** Whether comment entries are committed */
			bool _commentCommit;
		};
	}
}

#endif	/* __BE_IO_LOGSHEET_H__ */
