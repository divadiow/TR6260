#include "system.h"

bool lmac_check_ver_frame(uint8_t* frame) 
{ 
	return (frame[0] & 0x3); 
}

bool lmac_check_is_pv0_frame(uint8_t* frame) 
{ 
	return (lmac_check_ver_frame(frame) == FC_PV0); 
}

bool lmac_check_is_pv1_frame(uint8_t* frame) 
{ 
	return (lmac_check_ver_frame(frame) == FC_PV1); 
}

bool lmac_check_group_addr_frame(uint8_t *addr) 		 
{	
	return (addr[0] & 0x01);
}

bool lmac_check_broadc_addr_frame(uint8_t *addr) 		 
{	
	uint8_t base_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	return !(memcmp(addr,base_addr,6));
}

bool lmac_check_management_frame(GenericMacHeader *gmh)  
{	
	return (gmh->type == FC_PV0_TYPE_MGMT); 
}

bool lmac_check_control_frame(GenericMacHeader *gmh) 	
{	
	return (gmh->type == FC_PV0_TYPE_CTRL );
}

bool lmac_check_data_frame(GenericMacHeader *gmh) 		
{	
	return (gmh->type == FC_PV0_TYPE_DATA );
}

bool lmac_check_extension_frame(GenericMacHeader *gmh) 	
{	
	return (gmh->type == FC_PV0_TYPE_EXT );
}

bool lmac_check_qos_data_frame(GenericMacHeader *gmh)	
{	
	return ((gmh->type == FC_PV0_TYPE_DATA) && (gmh->subtype == FC_PV0_TYPE_DATA_QOS_DATA )); 
}

bool lmac_check_qos_null_frame(GenericMacHeader *gmh)  
{   
	return ((gmh->type == FC_PV0_TYPE_DATA) && (gmh->subtype == FC_PV0_TYPE_DATA_QOS_NULL )); 
}

bool lmac_check_probe_req_frame(GenericMacHeader *gmh) 	
{	
	return ((gmh->type == FC_PV0_TYPE_MGMT) && (gmh->subtype == FC_PV0_TYPE_MGMT_PROBE_REQ)); 
}

bool lmac_check_probe_rsp_frame(GenericMacHeader *gmh) 	
{	
	return ((gmh->type == FC_PV0_TYPE_MGMT) && (gmh->subtype == FC_PV0_TYPE_MGMT_PROBE_RSP)); 
}

bool lmac_check_pspoll_frame(GenericMacHeader *gmh) 
{	
	return ((gmh->type == FC_PV0_TYPE_CTRL) && (gmh->subtype == FC_PV0_TYPE_CTRL_PS_POLL)); 
}

bool lmac_check_retry_frame(GenericMacHeader *gmh)     
{   
	return (gmh->retry);
}

bool lmac_check_more_data_frame(GenericMacHeader *gmh)  
{   
	return (gmh->more_data);
}

bool lmac_check_beacon_frame(GenericMacHeader* gmh)    
{   
	return ((gmh->type == FC_PV0_TYPE_MGMT) && (gmh->subtype == FC_PV0_TYPE_MGMT_BEACON)); 
}

bool lmac_check_auth_frame(GenericMacHeader* gmh)      
{   
	return ((gmh->type == FC_PV0_TYPE_MGMT) && (gmh->subtype == FC_PV0_TYPE_MGMT_AUTH)); 
}
bool lmac_check_asso_frame(GenericMacHeader* gmh)      
{   
	return ((gmh->type == FC_PV0_TYPE_MGMT) && (gmh->subtype == FC_PV0_TYPE_MGMT_ASSOC_RSP)); 
}

bool lmac_check_nulldata_frame(GenericMacHeader* gmh)
{
	return ((gmh->type == FC_PV0_TYPE_DATA) && (gmh->subtype == FC_PV0_TYPE_DATA_DATA_NULL)); 

}
bool lmac_check_protected_frame(GenericMacHeader* gmh)  
{   
	return (gmh->protect); 
}

bool lmac_check_s1g_beacon_frame(GenericMacHeader *gmh) 
{ 	
	return ((gmh->type == FC_PV0_TYPE_EXT) && 	(gmh->subtype == FC_PV0_TYPE_EXT_S1G_BEACON));
}

bool lmac_check_fragmented_frame(GenericMacHeader* gmh) 
{   
	return (gmh->more_frag || gmh->fragment_number);
}

bool lmac_check_amsdu_frame(GenericMacHeader* gmh)
{
	if (lmac_check_qos_data_frame(gmh)) {
		return (bool) gmh->qos_amsdu_present;
	}

	return false;
}

bool lmac_check_eapol_frame(GenericMacHeader *gmh, int len)
{
	const uint8_t rfc1042_hdr[] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
	uint8_t *pos = gmh->payload;
	uint16_t ptype = 0;

	if (len < sizeof(*gmh) + sizeof(rfc1042_hdr) + 2)
		return false;

	if (!lmac_check_data_frame(gmh) || lmac_check_qos_null_frame(gmh))
		return false;

	if (lmac_check_qos_data_frame(gmh))
		pos += 2;

	if (memcmp(pos, rfc1042_hdr, sizeof(rfc1042_hdr) != 0))
		return false;

	pos += sizeof(rfc1042_hdr);
	ptype = *pos << 8 | *(pos + 1);

	return (ptype == ETH_P_PAE);
}
