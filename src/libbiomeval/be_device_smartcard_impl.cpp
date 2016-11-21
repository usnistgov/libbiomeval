/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <arpa/inet.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include "be_device_smartcard_impl.h"
#include <be_device_smartcard_apdu.h>
#include <be_memory_mutableindexedbuffer.h>

namespace BE = BiometricEvaluation;

BiometricEvaluation::Device::Smartcard::Impl::Impl(
    unsigned int cardNum)
{
	/* Exceptions float out */
	this->connectToCard(cardNum);
}

BiometricEvaluation::Device::Smartcard::Impl::Impl(
    unsigned int cardNum,
    const Memory::uint8Array &appID)
{
	if (appID.size() > APDU::MAX_NC_SIZE)
		throw (BE::Error::ParameterError("Application ID too large"));

	/* Exceptions float out */
	this->connectToCard(cardNum);

	BE::Device::Smartcard::APDU apdu{};
	apdu.cla        = 0x00;
        apdu.ins        = 0xA4;		/* SELECT command */
        apdu.p1         = 0x04;		/* command data is DF name */
        apdu.p2         = 0x00;
        apdu.lc         = appID.size();
        std::memcpy(apdu.nc, appID, appID.size());
        apdu.le         = 0x00;

	/*
	 * Set the Le field flag so any file control info block, if any,
	 * is returned.
	 */
        apdu.field_mask = APDU::FIELD_LC | APDU::FIELD_LE;

	try {
		auto response = this->sendAPDU(apdu);
		this->_lastResponseData = response.data;
	} catch (BE::Device::Smartcard::APDUException &e) {
		if ((e.response.sw1 == APDU::CHECK_ERR_WRONG_PARAM_QUAL) &&
		    (e.response.sw2 == APDU::FILE_OR_APP_NOT_FOUND)) {
			throw BE::Error::ParameterError(
			    "Application not found");
		} else {
			throw;
		}
	}

}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::Smartcard::Impl::getDedicatedFileObject(
    const Memory::uint8Array &objectID)
{
	if (objectID.size() > APDU::MAX_NC_SIZE)
		throw (BE::Error::ParameterError("Data object ID too large"));

	BE::Device::Smartcard::APDU apdu{};
	apdu.cla        = 0x00;
	apdu.ins        = 0xCB;		/* P1-P2 contains file ID */
	apdu.p1         = 0x3F;		/* Use the current dedicated file */
	apdu.p2         = 0xFF;
	apdu.lc         = objectID.size();
	std::memcpy(apdu.nc, objectID, objectID.size());
	apdu.le         = 0x00;
	apdu.field_mask = APDU::FIELD_LC | APDU::FIELD_LE;

	/* We could be check sw1/sw2, but it's not clear what the status
	 * words mean. 0x6A80 (incorrect parameters) is often returned when
	 * a 'known' object tag is sent over, while while 0x6A82 is returned
	 * returned when an unknown tag is sent over. So let the APDUException
	 * float out of here, and the app can examine the status words.
	 */
	auto response = this->sendAPDU(apdu);
	return (response.data);
} 

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::Smartcard::Impl::getLastAPDU() const
{
	return (this->_lastAPDU);
}

BiometricEvaluation::Memory::uint8Array
BiometricEvaluation::Device::Smartcard::Impl::getLastResponseData() const
{
	return (this->_lastResponseData);
}

void
BiometricEvaluation::Device::Smartcard::Impl::setDryrun(bool state)
{
	this->_dryrun = state;
}

/*
 * Local functions.
 */

static void
getAPDUResponse(
    SCARDHANDLE hCard,
    SCARD_IO_REQUEST pioSendPci, 
    uint8_t *recvBuf, DWORD recvLen,
    BE::Memory::uint8Array &response, uint8_t &sw1, uint8_t &sw2)
{
	LONG rc;
	uint8_t lsw1, lsw2;
	uint8_t bGetRes[5] = {0x00, 0xC0, 0x00, 0x00, 0x00};
	DWORD lRecvLen;

	response.resize(BE::Device::Smartcard::APDU::MAX_LE_SIZE);
	BE::Memory::MutableIndexedBuffer indexedResponse(response);

	/* Get the response status words and check whether chaining was done */
	lsw1 = recvBuf[recvLen - 2];
	lsw2 = recvBuf[recvLen - 1];

	/* Handle response chaining */
	lRecvLen = recvLen;
	while (lsw1 == BE::Device::Smartcard::APDU::NORMAL_CHAINING) {
		indexedResponse.push(recvBuf, lRecvLen - 2);
		bGetRes[4] = lsw2;	/* The Le field */
		lRecvLen = (0 == lsw2) ? 256 : lsw2;
		lRecvLen += 2;	/* Account for the SW */
		rc = SCardTransmit(hCard, &pioSendPci, bGetRes, 5, nullptr,
		    recvBuf, &lRecvLen);

		if (rc != SCARD_S_SUCCESS) {
			throw BE::Error::StrategyError(
			    "Transmit of GET RESPONSE: "
			    + std::string(pcsc_stringify_error(rc)));
		}
		lsw1 = recvBuf[lRecvLen - 2];
		lsw2 = recvBuf[lRecvLen - 1];
	}
	sw1 = lsw1;
	sw2 = lsw2;
	indexedResponse.push(recvBuf, lRecvLen - 2);
	response.resize(indexedResponse.getIndex());
}

/*
 * Private methods.
 */

/*
 * Deleter for smart pointers wrapping C-allocated objects.
 */
auto freeCStyle = [](void *ptr) {free(ptr);};

/*
 */
void
BiometricEvaluation::Device::Smartcard::Impl::connectToCard(
    unsigned int cardNum)
{
	LONG rc;

	SCARDCONTEXT context;
	if (SCardEstablishContext(
	    SCARD_SCOPE_SYSTEM, nullptr, nullptr, &context) !=
	    SCARD_S_SUCCESS) {
		throw BE::Error::StrategyError(
		    "Could not establish PCSC context");
	}

	/* Retrieve the available readers list */
	DWORD dwReaders;
	rc = SCardListReaders(context, nullptr, nullptr, &dwReaders);
	if (rc != SCARD_S_SUCCESS) {
		throw BE::Error::StrategyError(
		    "Could not list readers: "
		    + std::string(pcsc_stringify_error(rc)));
	}
	LPTSTR mszReaders;
	mszReaders = (LPTSTR)malloc(sizeof(char)*dwReaders);
	if (mszReaders == nullptr) {
		throw BE::Error::MemoryError("Reader string array");
	}
	std::unique_ptr<void, decltype(freeCStyle)>
	    pmszReaders(mszReaders, freeCStyle);
	rc = SCardListReaders(context, nullptr, mszReaders, &dwReaders);
	if (rc != SCARD_S_SUCCESS) {
		throw BE::Error::StrategyError(
		    "Could not list readers: "
		    + std::string(pcsc_stringify_error(rc)));
	}
	/*
	 * Extract readers from the nul separated set of strings.
	 */
	std::vector<std::string> readers;
	char *ptr = mszReaders;
	while (*ptr != '\0') {
		readers.push_back(std::string(ptr));
		ptr += std::strlen(ptr)+1;
	}
	if (readers.size() == 0) {
		throw BE::Error::StrategyError("No readers found");
	}
	SCARDHANDLE hCard;
	DWORD rdrprot;
	unsigned int readerNum = 0;
	bool found = false;
	for (readerNum = 0; readerNum < readers.size(); readerNum++) {
		rc = SCardConnect(context, readers[readerNum].c_str(),
		     SCARD_SHARE_EXCLUSIVE,
		     SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		     &hCard, &rdrprot);
		if (rc == 0) {
			if (readerNum == cardNum) {
				this->_hCard = hCard;
				this->_readerID = readers[readerNum];
				found = true;
				break;
			} else {
				SCardDisconnect(hCard, SCARD_RESET_CARD);
			}
		} else {
			/*
			 * We could throw an exception here, letting the
			 * the caller know that the card is missing or
			 * otherwise inaccessible, but they probably
			 * don't care that a reader/card they didn't ask
			 * for has an issue.
			 */
		}
	}
	if (!found) {
		throw BE::Error::ParameterError(
		    "No card found for given number");
	}
}

/*
 */
void
BiometricEvaluation::Device::Smartcard::Impl::sendChained(
    SCARD_IO_REQUEST pioSendPci,
    BE::Device::Smartcard::APDU &apdu,
    BE::Memory::uint8Array &response,
    uint8_t &sw1, uint8_t &sw2)
{
	LONG rc;
	int LcLen;
	int maxLcLen;
	int ncIndex;
	uint8_t bSendBuffer[MAX_BUFFER_SIZE];
	uint8_t bRecvBuffer[MAX_BUFFER_SIZE];
	DWORD sendIndex;
	DWORD recvLength;

	recvLength = sizeof(bRecvBuffer);

	bSendBuffer[0] = apdu.cla;
	bSendBuffer[1] = apdu.ins;
	bSendBuffer[2] = apdu.p1;
	bSendBuffer[3] = apdu.p2;

	LcLen = apdu.lc;
	maxLcLen = APDU::MAX_SHORT_LC - APDU::HEADER_LEN - APDU::FLEN_TRAILER;
	if ((apdu.field_mask & APDU::FIELD_LC) != 0) {
		maxLcLen -= APDU::FLEN_LC_SHORT;
	}

	if ((apdu.field_mask & APDU::FIELD_LE) != 0) {
		if (apdu.le > APDU::MAX_SHORT_LE) {
			throw BE::Error::ParameterError(
			    "Invalid Le value");
		}
		maxLcLen -= APDU::FLEN_LE_SHORT;
	}
	ncIndex = 0;

	unsigned int lastAPDUIndex = 0;
	while (LcLen > 0) {
		sendIndex = 4;
		if ((apdu.field_mask & APDU::FIELD_LC) != 0) {
			if (LcLen > maxLcLen) {
				bSendBuffer[0] |= APDU::FLAG_CLA_CHAIN;
				bSendBuffer[sendIndex] = (uint8_t)maxLcLen;
				sendIndex += 1;
				std::memcpy(&bSendBuffer[sendIndex],
				    &apdu.nc[ncIndex], maxLcLen);
				ncIndex += maxLcLen;
				sendIndex += maxLcLen;
				LcLen -= maxLcLen;
			} else {	/* Note that LcLen cannot be 0 here */
				bSendBuffer[0] &= ~APDU::FLAG_CLA_CHAIN;
				bSendBuffer[sendIndex] = (uint8_t)LcLen;
				sendIndex += 1;
				std::memcpy(&bSendBuffer[sendIndex],
				    &apdu.nc[ncIndex], LcLen);
				ncIndex += LcLen;
				sendIndex += LcLen;
				LcLen = 0;
			}
		}
		if ((apdu.field_mask & APDU::FIELD_LE) != 0) {
			bSendBuffer[sendIndex] = (uint8_t)apdu.le;
			sendIndex += 1;
		}

		/*
		 * At this point, sendIndex is the location of where the next
		 * byte would be written into the send buffer, and that happens
		 * to be the current length of the send buffer as well.
		 * Append the send buffer to the saved last APDU.
		 */
		this->_lastAPDU.resize(lastAPDUIndex + sendIndex);
		std::memcpy(
		    &this->_lastAPDU[lastAPDUIndex], bSendBuffer, sendIndex);
		lastAPDUIndex += sendIndex;
		this->_lastResponseData.resize(0);
		if (this->_dryrun == false) {
			rc = SCardTransmit(this->_hCard,
			    &pioSendPci, bSendBuffer,
			    sendIndex, nullptr, bRecvBuffer, &recvLength);
			if (rc != SCARD_S_SUCCESS) {
				throw BE::Error::StrategyError(
				    "Transmit failed: "
				    + std::string(pcsc_stringify_error(rc)));
			}
			getAPDUResponse(this->_hCard, pioSendPci,
			    bRecvBuffer, recvLength, response,
			    sw1, sw2);
			this->_lastResponseData = response;
			/*
			 * If we fail in the middle of the chain, send no more
			 * data. The client can retrieve the response and
			 * status.
			 */
			if (sw1 != APDU::NORMAL_COMPLETE) {
				LcLen = 0;	/* Send no more data */
			}
		} else {
			sw1 = APDU::NORMAL_COMPLETE;
			sw2 = 0;
		}
	}
}

/*
 */
void
BiometricEvaluation::Device::Smartcard::Impl::sendExtended(
    SCARD_IO_REQUEST pioSendPci,
    BE::Device::Smartcard::APDU &apdu,
    BE::Memory::uint8Array &response,
    uint8_t &sw1, uint8_t &sw2)
{
	LONG rc;
	bool lcle_extended;
	uint8_t bSendBuffer[MAX_BUFFER_SIZE_EXTENDED];
	uint8_t bRecvBuffer[MAX_BUFFER_SIZE_EXTENDED];
	DWORD sendIndex;
	DWORD recvLength;

	recvLength = sizeof(bRecvBuffer);

	bSendBuffer[0] = apdu.cla;
	bSendBuffer[1] = apdu.ins;
	bSendBuffer[2] = apdu.p1;
	bSendBuffer[3] = apdu.p2;
	sendIndex = 4;
	/*
	 * The Lc and Le fields are 0, 1, or 3 bytes. If the length is present
	 * and will fit in one byte, store it; else, store a '00' byte, then
	 * the value. Also, if either field is extended, then both must be.
	 * (Note that we are assuming the card can accept extended fields;
	 * we really should check the card capabilities third table, if that's
	 * even present. Of course, it may not be, but that is also where the
	 * command chaining indicator is located; 7816-4 is just wonderful.
	 * You're probably better off just shoving data to the card and
	 * hoping its tiny little head doesn't explode; that can be detected.)
	 */
	if ((apdu.lc > APDU::MAX_SHORT_LC) ||
	    (apdu.le > APDU::MAX_SHORT_LE)) {
		lcle_extended = true;
	} else {
		lcle_extended = false;
	}
	if ((apdu.field_mask & APDU::FIELD_LC) != 0) {
		if (lcle_extended) {
			bSendBuffer[sendIndex] = 0;
			uint16_t val = (uint16_t)htons(apdu.lc);
			std::memcpy(bSendBuffer + sendIndex+1, &val, 2);
			sendIndex += 3;
		} else {
			bSendBuffer[sendIndex] = (uint8_t)apdu.lc;
			sendIndex += 1;
		}
		std::memcpy(bSendBuffer + sendIndex, apdu.nc, apdu.lc);
		sendIndex += apdu.lc;
	}
	if ((apdu.field_mask & APDU::FIELD_LE) != 0) {
		if (lcle_extended) {
			bSendBuffer[sendIndex] = 0;
			uint16_t val = (uint16_t)htons(apdu.le);
			std::memcpy(bSendBuffer + sendIndex+1, &val, 2);
			sendIndex += 3;
		} else {
			bSendBuffer[sendIndex] = (uint8_t)apdu.le;
			sendIndex += 1;
		}
	}
	/*
	 * At this point, sendIndex is the location of where the next
	 * byte would be written into the send buffer, and that happens
	 * to be the current length of the send buffer as well.
	 * Copy the send buffer to the saved last APDU.
	 */
	this->_lastAPDU.resize(sendIndex);
	this->_lastAPDU.copy(bSendBuffer, sendIndex);
	this->_lastResponseData.resize(0);
	if (this->_dryrun == false) {
		rc = SCardTransmit(this->_hCard, &pioSendPci,
		    bSendBuffer, sendIndex,
		    nullptr, bRecvBuffer, &recvLength);
		if (rc != SCARD_S_SUCCESS) {
			throw BE::Error::StrategyError(
			    "Transmit failed: "
			    + std::string(pcsc_stringify_error(rc)));
		}
		getAPDUResponse(this->_hCard, pioSendPci,
		    bRecvBuffer, recvLength, response, sw1, sw2);
		this->_lastResponseData = response;
	} else {
		sw1 = APDU::NORMAL_COMPLETE;
		sw2 = 0;
	}
}

BiometricEvaluation::Device::Smartcard::APDUResponse
BiometricEvaluation::Device::Smartcard::Impl::sendAPDU(
    Device::Smartcard::APDU &apdu)
{
	LONG rc;
 	SCARD_IO_REQUEST pioSendPci;
	DWORD dwActiveProtocol;

	if (this->_dryrun == true) {
		dwActiveProtocol = SCARD_PROTOCOL_T0;
	} else {
		/* connect to a reader (even without a card) */
		dwActiveProtocol = -1;
		rc = SCardReconnect(this->_hCard, SCARD_SHARE_EXCLUSIVE,
		    SCARD_PROTOCOL_T0|SCARD_PROTOCOL_T1, SCARD_LEAVE_CARD,
		    &dwActiveProtocol);
		if (rc != SCARD_S_SUCCESS)
		    throw BE::Error::StrategyError("SCardReconnect failed: "
		    + std::string(pcsc_stringify_error(rc)));
	}
	switch(dwActiveProtocol) {
		case SCARD_PROTOCOL_T0:
			pioSendPci = *SCARD_PCI_T0;
			break;
		case SCARD_PROTOCOL_T1:
			pioSendPci = *SCARD_PCI_T1;
			break;
		default:
			throw BE::Error::StrategyError("Unknown card protocol");
	}

	if (this->_dryrun == false) {
		rc = SCardBeginTransaction(this->_hCard);
		if (rc != SCARD_S_SUCCESS) {
			(void)SCardEndTransaction(this->_hCard,
			    SCARD_LEAVE_CARD);
			throw BE::Error::StrategyError(
			    "SCardBeginTransaction failed: "
			    + std::string(pcsc_stringify_error(rc)));
		}
	}
	Memory::uint8Array response;
	uint8_t sw1, sw2;
	if (dwActiveProtocol == SCARD_PROTOCOL_T0)
		this->sendChained(
		    pioSendPci, apdu, response, sw1, sw2);
	else
		this->sendExtended(
		    pioSendPci, apdu, response, sw1, sw2);

	if (sw1 != APDU::NORMAL_COMPLETE) {
		throw BE::Device::Smartcard::APDUException(
		    BE::Device::Smartcard::APDUResponse(response, sw1, sw2),
		    this->_lastAPDU);
	}
	if (this->_dryrun == false) {
		rc = SCardEndTransaction(this->_hCard, SCARD_LEAVE_CARD);
		if (rc != SCARD_S_SUCCESS) {
			throw BE::Error::StrategyError("End Transaction failed:"
			+ std::string(pcsc_stringify_error(rc)));
		}
	}
	return (BE::Device::Smartcard::APDUResponse(response, sw1, sw2));
}

std::string
BiometricEvaluation::Device::Smartcard::Impl::getReaderID() const
{
	return (this->_readerID);
}

BiometricEvaluation::Device::Smartcard::Impl::~Impl()
{
	SCardDisconnect(this->_hCard, SCARD_RESET_CARD);
}

