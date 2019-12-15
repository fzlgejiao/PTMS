#include "ireader.h"
#include "itag.h"
#include "irdm.h"
#include "CModbus.h"

iReader::iReader(QObject *parent)
	: QObject(parent)
{
	RDM = (iRDM *)parent;
	tmrReader = new TMR_Reader();
	bCreated = false;
	cur_plan = 0;
}

iReader::~iReader()
{
	TMR_destroy(tmrReader);
}
void iReader::checkerror()
{
	//if (ret != TMR_SUCCESS)
	{
		QString errormessage = QString(TMR_strerr(tmrReader, ret));
		qDebug() << "Error reader-init : FAILED - " << errormessage;
	}
}
bool iReader::RD_init(bool force)
{	
	//check if need to create reader
	if (bCreated)
	{		
		if (m_uri == RDM->RDM_comname && force==false)
			return true;
		TMR_destroy(tmrReader);
	}

	m_uri = RDM->RDM_comname;
	ret = TMR_create(tmrReader, m_uri.toStdString().c_str());
	if (ret != TMR_SUCCESS)
		return false;
	else
		bCreated = true;
	ret = TMR_connect(tmrReader);
	if (ret != TMR_SUCCESS) return false;

	//Get reader's  hardware ,software, model type
	TMR_String str;
	char string[50];
	str.value = string;
	str.max = 50;

	ret = TMR_paramGet(tmrReader, TMR_PARAM_PRODUCT_GROUP, &str);
	if (ret != TMR_SUCCESS) return false;
	group = QString(str.value);

	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_HARDWARE, &str);
	if (ret != TMR_SUCCESS) return false;
	hardware = QString(str.value);

	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_SOFTWARE, &str);
	if (ret != TMR_SUCCESS) return false;
	software = QString(str.value);

	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_MODEL, &str);
	if (ret != TMR_SUCCESS) return false;
	modleversion = QString(str.value);

	//set parameters to reader
	TMR_Region region = TMR_REGION_PH;
	int power = 3000;
	bool is_send_sl = true;
	int t4 = 3000;
	//bool uniquebydata = true;									//if one simple plan ,not necessary publish by data
	bool checkport = true;
	TMR_GEN2_Session session = TMR_GEN2_SESSION_S0;
	TMR_GEN2_Tari tari = TMR_GEN2_TARI_25US;
	TMR_GEN2_TagEncoding tagcoding = TMR_GEN2_MILLER_M_4;

	ret = TMR_paramSet(tmrReader, TMR_PARAM_REGION_ID, &region);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_RADIO_READPOWER, &power);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_RADIO_WRITEPOWER, &power);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_SEND_SELECT, &is_send_sl);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_T4, &t4);
	if (ret != TMR_SUCCESS) return false;
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_TAGREADDATA_UNIQUEBYDATA, &uniquebydata);
	//if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_ANTENNA_CHECKPORT, &checkport);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_SESSION, &session);
	if (ret != TMR_SUCCESS) return false;
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_TARI, &tari);
	//if (ret != TMR_SUCCESS) return false;		
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_TAGENCODING, &tagcoding);
	//if (ret != TMR_SUCCESS) return false;	

		
	//Auto check connected antennas
	quint8 antennaCount = 2;		//M6E micro support 2 antenna
	antennaList[0] = 1;
	antennaList[1] = 2;
	quint8 antennalist[2];
/*
	TMR_uint8List connectedantennalist;
	connectedantennalist.list = antennalist;
	connectedantennalist.max = sizeof(antennalist);
	connectedantennalist.len = 0;

	ret = TMR_paramGet(tmrReader, TMR_PARAM_ANTENNA_CONNECTEDPORTLIST, &connectedantennalist);
	if (ret != TMR_SUCCESS) return false;

	//update antennas
	if (connectedantennalist.len == 0) return false;
	antennaCount = connectedantennalist.len;
	memcpy(antennaList, connectedantennalist.list, antennaCount);
*/
	//set OP antenna,use the first one
	antennaCount = 1;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_TAGOP_ANTENNA, &antennaList[0]);
	if (ret != TMR_SUCCESS) return false;

	for (int i = 0; i<PLAN_CNT; i++)
	{
		ret = TMR_RP_init_simple(&subplan[i], antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 0);
		if (ret != TMR_SUCCESS) return false;
		subplanPtrs[i] = &subplan[i];
	}

	int subplanindex = 0;

	//plan to Read On-chip RSSI
	OC_rssi_mask = 0x1F;
	ret = TMR_TF_init_gen2_select(&OC_rssi_select, false, TMR_GEN2_BANK_USER, 0xD0, 8, &OC_rssi_mask);
	if (ret != TMR_SUCCESS) return false;
	OC_rssi_select.u.gen2Select.target = SELECT;
	OC_rssi_select.u.gen2Select.action = ON_N_OFF;

	ret = TMR_TagOp_init_GEN2_ReadData(&OC_rssi_read, TMR_GEN2_BANK_RESERVED, 0xD, 1);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_RP_set_filter(&subplan[subplanindex], &OC_rssi_select);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_RP_set_tagop(&subplan[subplanindex], &OC_rssi_read);
	if (ret != TMR_SUCCESS) return false;
	subplanindex++;

	//plan to Read tag temperature	
	ret = TMR_TF_init_gen2_select(&tempselect, false, TMR_GEN2_BANK_USER, 0xE0, 0, 0);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_TagOp_init_GEN2_ReadData(&tempread, TMR_GEN2_BANK_RESERVED, 0x0E, 1);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_RP_set_filter(&subplan[subplanindex], &tempselect);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_RP_set_tagop(&subplan[subplanindex], &tempread);
	if (ret != TMR_SUCCESS) return false;
	subplanindex++;

	//ret = TMR_RP_init_multi(&multiplan, subplanPtrs, PLAN_CNT, 0);
	//if (ret != TMR_SUCCESS) return false;
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &multiplan);
	//if (ret != TMR_SUCCESS) return false;


	//set async read mode	
	//setCReader();
	//TMR_ReadListener ll = std::bind(readcallback)

	//TMR_ReadExceptionListener exceptionlistner = exceptioncallback;


	//func = (myreadcallback)&readcallback;

	//rlb.listener = CReader::readcallback;
	//rlb.cookie = NULL;
	//reb.listener = &exceptioncallback;
	//reb.cookie = NULL;

	//ret = TMR_addReadListener(reader, &rlb);
	//if (ret != TMR_SUCCESS) goto Failed;
	//ret = TMR_addReadExceptionListener(reader, &reb);
	//if (ret != TMR_SUCCESS) goto Failed;
	//ret = TMR_startReading(reader);
	//if (ret != TMR_SUCCESS) goto Failed;
	return true;
}
bool iReader::wirteEpc(const QByteArray& epc_old, const QString& epc_new)
{
	TMR_TagData		oldepc;
	TMR_TagData		newepc;
	TMR_TagOp		tagop;
	TMR_TagFilter	filter;
	TMR_Status		ret;

	oldepc.epcByteCount = epc_old.count();
	memcpy(oldepc.epc, epc_old.data(), epc_old.count());

	newepc.epcByteCount = epc_new.count();
	memcpy(newepc.epc, epc_new.toStdString().c_str(), epc_new.count());

	ret = TMR_TagOp_init_GEN2_WriteTag(&tagop, &newepc);
	if (ret != TMR_SUCCESS) 
		return false;
	ret = TMR_TF_init_tag(&filter, &oldepc);
	if (ret != TMR_SUCCESS) 
		return false;

	/* Execute the tag operation Gen2 writeTag with select filter applied*/
	ret = TMR_executeTagOp(tmrReader, &tagop, &filter, NULL);
	if (ret == TMR_SUCCESS) 
		return true;
	else
	{
		qDebug() << "Error reader-wirteEpc : FAILED - " << QString(TMR_strerr(tmrReader, ret));
		return false;
	}
}

quint64 iReader::readtagTid(TMR_TagFilter *filter)
{
	TMR_TagOp		readTid;
	TMR_uint8List	tiddata;
	TMR_Status		ret;
	quint8			buff[32];

	tiddata.max = sizeof(buff);
	tiddata.list = buff;
	tiddata.len = 0;
	//read all TID BANK
	ret = TMR_TagOp_init_GEN2_ReadData(&readTid, TMR_GEN2_BANK_TID, 0, 0);
	if (ret != TMR_SUCCESS) return 0;
	ret = TMR_executeTagOp(tmrReader, &readTid, filter, &tiddata);
	if (ret != TMR_SUCCESS) return 0;
	QByteArray byts((char *)tiddata.list, tiddata.len);

	if (tiddata.len < 8) return 0;

	QByteArray idbytes;
	quint8 b0 = byts[0];		//Epc TID first byte may be 0xE0 or 0xE2 in Gen2

	if (b0 == 0xE2)
		idbytes = byts.right(8);
	else if (b0 == 0xE0)
		idbytes = byts.left(8);
	else return 0;

	return bytes2longlong(idbytes);
}
quint64 iReader::readtagCalibration(TMR_TagFilter *filter)
{
	TMR_TagOp		readcalibration;
	TMR_uint8List	calibrationdata;
	TMR_Status		ret;
	quint8 buff[32];

	calibrationdata.max = sizeof(buff);
	calibrationdata.list = buff;
	calibrationdata.len = 0;
	//In UserBank  ----word offset=0x8, word length=4
	ret = TMR_TagOp_init_GEN2_ReadData(&readcalibration, TMR_GEN2_BANK_USER, 0x8, 4);
	if (ret != TMR_SUCCESS) return 0;
	ret = TMR_executeTagOp(tmrReader, &readcalibration, filter, &calibrationdata);
	if (ret != TMR_SUCCESS) return 0;

	QByteArray Calibratebytes((char *)calibrationdata.list, calibrationdata.len);

	return bytes2longlong(Calibratebytes);
}
void iReader::readtag()
{
	TMR_Status		ret;
	
	//clear old online tags before read again
	RDM->tagOnline.clear();
	if (!switchplans()) return;


	//--------------------------test code---------------------
	//quint64 tid1 = 1;
	//for (uchar i = 0; i < 16; i++)
	//{
	//	tid1 = i;
	//	QByteArray epc1;
	//	epc1.append('1'+i);
	//	RDM->tagOnline.insert(tid1, epc1);
	//}
	//--------------------------test code---------------------
	int readCount = 0;
	ret = TMR_read(tmrReader, RD_TIMEOUT, &readCount);

	if (ret != TMR_SUCCESS)
	{
		QString errormessage = QString(TMR_strerr(tmrReader, ret));
		qDebug() << "Error reader-readtag : FAILED - " << errormessage;
		qDebug() << QString("Status Error Code=%1").arg(ret, 16, 16);

		RD_init(true);
		RDM->getCModbus()->MB_init();

		return;
	}
	qDebug() << "Info reader-readtag : SUCCESS	ReadCounts=" << readCount;

	while (TMR_SUCCESS == TMR_hasMoreTags(tmrReader))
	{
		TMR_TagReadData trd;
		//prepare data buff
		//quint8 dataBuff[256];
		quint8 databuffer[4];
		//ret = TMR_TRD_init_data(&trd, sizeof(dataBuff), dataBuff);		
		
		trd.data.max = sizeof(databuffer);
		trd.data.list = databuffer;
		trd.data.len = 0;

		ret = TMR_getNextTag(tmrReader, &trd);
		if (ret != TMR_SUCCESS) 
			continue;

		//read tag ok!
		TMR_TagFilter epcfilter;
		ret = TMR_TF_init_tag(&epcfilter, &trd.tag);
		if (ret != TMR_SUCCESS)
			continue;
			
		if (trd.data.len > 0)
		{
			//get Tid ,epc		
			quint64 tid = readtagTid(&epcfilter);
			if (tid == 0) 
				continue;
			QByteArray tEPC((char *)trd.tag.epc, trd.tag.epcByteCount);
			
			//add tag into online list
			RDM->tagOnline.insert(tid, tEPC);

			iTag * tag = RDM->Tag_get(tid);
			if (tag )
			{
				tag->T_ticks = TAG_TICKS;
				tag->T_alarm_offline = false;
				tag->T_epc = tEPC;
				tag->T_rssi = trd.rssi;
				tag->T_data_flag |= Tag_Online; 
				if (tag->T_caldata.all == 0)
					tag->T_caldata.all = readtagCalibration(&epcfilter);

				quint8 data0 = trd.data.list[0];	//use the data0 check the OC-Rssi plan or temperature plan
				if (data0 != 0)
				{
					ushort temperaturecode = (trd.data.list[0] << 8) + trd.data.list[1];
					if (temperaturecode > 0)
					{
						float Temp = tag->parseTCode(temperaturecode);
						if (tag->T_temp == 0.0														//init temperature
							|| (qAbs(Temp - tag->T_temp) < qAbs(tag->T_temp)*0.5))					//reasonable temperature
						{
							tag->T_temp = Temp;
							if (tag->T_temp > tag->T_uplimit)										//bigger than up limit
								tag->T_alarm_temperature = true;
							qDebug()<< "managed tag : sid = " << tag->T_sid
									<< " uid = " << tag->T_uid
									<< " epc = " << tag->T_epc
									<< " rssi = " << tag->T_rssi
									<< " temperature = " << tag->T_temp
									<< " temp_alarmed = " << tag->T_alarm_temperature;
						}
					}
				}
				else 
				{
					tag->T_OC_rssi = trd.data.list[1];

					qDebug()<< "managed tag : sid = " << tag->T_sid
							<< " uid = " << tag->T_uid
							<< " epc = " << tag->T_epc
							<< " rssi = " << tag->T_rssi
							<< " Oc-rssi = " << tag->T_OC_rssi
							<< "frequency =" << trd.frequency;
				}
				emit tagUpdated(tag);
			}
			else
			{
				qDebug() << "unknown tag : uid = " << tid << "epc = " << tEPC;
			}

		}
	}
	qDebug() << "Online tags count: " << RDM->tagOnline.count();

}
void iReader::readcallback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie)
{
	char epcStr[128];
	quint64 tagid;
	quint8 temperatureBuff[4];
	TMR_TagReadData hi;
	TMR_Status ret;

	ret = TMR_TRD_init_data(&hi, sizeof(temperatureBuff), temperatureBuff);

	TMR_bytesToHex(t->tag.epc, t->tag.epcByteCount, epcStr);

	float tt = 12.6f;

	//emit currenttemp(tt);
}
void iReader::exceptioncallback(TMR_Reader *reader, TMR_Status error, void *cookie)
{
	QString errormessage = QString(TMR_strerr(reader, error));
}
quint64 iReader::bytes2longlong(QByteArray& bytes)
{
	if (bytes.length() != 8) return 0;

	quint64 d0 = bytes[0] & 0xff;
	quint64 d1 = bytes[1] & 0xff;
	quint64 d2 = bytes[2] & 0xff;
	quint64 d3 = bytes[3] & 0xff;
	quint64 d4 = bytes[4] & 0xff;
	quint64 d5 = bytes[5] & 0xff;
	quint64 d6 = bytes[6] & 0xff;
	quint64 d7 = bytes[7] & 0xff;
	d0 <<= 56;
	d1 <<= 48;
	d2 <<= 40;
	d3 <<= 32;
	d4 <<= 24;
	d5 <<= 16;
	d6 <<= 8;
	return (d0 | d1 | d2 | d3 | d4 | d5 | d6 | d7);
}
bool iReader::switchplans()
{
	if (cur_plan == PLAN_CNT)
	{
		cur_plan = 0;
	}
	if (!tmrReader->connected) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &subplan[cur_plan]);
	if (ret == TMR_SUCCESS)
	{
		cur_plan++;
		return true;
	}
	else
		return false;
}