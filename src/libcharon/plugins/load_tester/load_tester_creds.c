/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "load_tester_creds.h"

#include <time.h>
#include <sys/stat.h>

#include <daemon.h>
#include <credentials/keys/shared_key.h>
#include <credentials/certificates/x509.h>
#include <utils/identification.h>

typedef struct private_load_tester_creds_t private_load_tester_creds_t;

/**
 * Private data of an load_tester_creds_t object
 */
struct private_load_tester_creds_t {
	/**
	 * Public part
	 */
	load_tester_creds_t public;

	/**
	 * Private key to create signatures
	 */
	private_key_t *private;

	/**
	 * CA certificate, to issue/verify peer certificates
	 */
	certificate_t *ca;

	/**
	 * Trusted CA certificates, including issuer CA
	 */
	linked_list_t *cas;

	/**
	 * Digest algorithm to issue certificates
	 */
	hash_algorithm_t digest;

	/**
	 * serial number to issue certificates
	 */
	u_int32_t serial;

	/**
	 * Preshared key for IKE
	 */
	shared_key_t *psk;

	/**
	 * Password for EAP
	 */
	shared_key_t *pwd;
};

/**
 * 1024-bit RSA key:
-----BEGIN RSA PRIVATE KEY-----
MIICXQIBAAKBgQDQXr7poAPYZLxmTCqR51STGRuk9Hc5SWtTcs6b2RzpnP8EVRLx
JEVxOKE9Mw6n7mD1pNrupCpnpGRdLAV5VznTPhSQ6k7ppJJrxosRYg0pHTZqBUEC
7nQFwAe10g8q0UnM1wa4lJzGxDH78d21cVweJgbkxAeyriS0jhNs7gO5nQIDAQAB
AoGACVACtkxJf7VY2jWTPXwaQoy/uIqYfX3zhwI9i6eTbDlxCE+JDi/xzpKaWjLa
99RmjvP0OPArWQB239ck03x7gAm2obutosGbqbKzJZS5cyIayzyW9djZDHBdt9Ho
quKB39aspWit3xPzkrr+QeIkiggtmBKALTBxTwxAU+P6euECQQD4IPdrzKbCrO79
LKvoPrQQtTjL6ogag9rI9n2ZuoK3/XVybh2byOXT8tA5G5jSz9Ac8XeVOsnH9gT5
3WXeaLOFAkEA1vrm/hVSEasp5eATgQ7ig9CF+GGKqhTwXp/uOSl/h3IRmStu5J0C
9AkYyx0bn3j5R8iUEX/C00KSE1kQNh4NOQJAVOsLYlRG2idPH0xThQc4nuM2jes1
K0Xm8ZISSDNhm1BeCoyPC4rExTW7d1/vfG5svgsRrvvQpOOYrl7MB0Lz9QJBALhg
AWJiyLsskEd90Vx7dpvUaEHo7jMGuEx/X6GYzK5Oj3dNP9NEMfc4IhJ5SWqRJ0KA
bTVA3MexLXT4iqXPSkkCQQDSjLhBwvEnSuW4ElIMzBwLbu7573z2gzU82Mj6trrw
Osoox/vmcepT1Wjy4AvPZHgxp7vEXNSeS+M5L29QNTp8
-----END RSA PRIVATE KEY-----
 */
static char private[] = {
  0x30,0x82,0x02,0x5d,0x02,0x01,0x00,0x02,0x81,0x81,0x00,0xd0,0x5e,0xbe,0xe9,0xa0,
  0x03,0xd8,0x64,0xbc,0x66,0x4c,0x2a,0x91,0xe7,0x54,0x93,0x19,0x1b,0xa4,0xf4,0x77,
  0x39,0x49,0x6b,0x53,0x72,0xce,0x9b,0xd9,0x1c,0xe9,0x9c,0xff,0x04,0x55,0x12,0xf1,
  0x24,0x45,0x71,0x38,0xa1,0x3d,0x33,0x0e,0xa7,0xee,0x60,0xf5,0xa4,0xda,0xee,0xa4,
  0x2a,0x67,0xa4,0x64,0x5d,0x2c,0x05,0x79,0x57,0x39,0xd3,0x3e,0x14,0x90,0xea,0x4e,
  0xe9,0xa4,0x92,0x6b,0xc6,0x8b,0x11,0x62,0x0d,0x29,0x1d,0x36,0x6a,0x05,0x41,0x02,
  0xee,0x74,0x05,0xc0,0x07,0xb5,0xd2,0x0f,0x2a,0xd1,0x49,0xcc,0xd7,0x06,0xb8,0x94,
  0x9c,0xc6,0xc4,0x31,0xfb,0xf1,0xdd,0xb5,0x71,0x5c,0x1e,0x26,0x06,0xe4,0xc4,0x07,
  0xb2,0xae,0x24,0xb4,0x8e,0x13,0x6c,0xee,0x03,0xb9,0x9d,0x02,0x03,0x01,0x00,0x01,
  0x02,0x81,0x80,0x09,0x50,0x02,0xb6,0x4c,0x49,0x7f,0xb5,0x58,0xda,0x35,0x93,0x3d,
  0x7c,0x1a,0x42,0x8c,0xbf,0xb8,0x8a,0x98,0x7d,0x7d,0xf3,0x87,0x02,0x3d,0x8b,0xa7,
  0x93,0x6c,0x39,0x71,0x08,0x4f,0x89,0x0e,0x2f,0xf1,0xce,0x92,0x9a,0x5a,0x32,0xda,
  0xf7,0xd4,0x66,0x8e,0xf3,0xf4,0x38,0xf0,0x2b,0x59,0x00,0x76,0xdf,0xd7,0x24,0xd3,
  0x7c,0x7b,0x80,0x09,0xb6,0xa1,0xbb,0xad,0xa2,0xc1,0x9b,0xa9,0xb2,0xb3,0x25,0x94,
  0xb9,0x73,0x22,0x1a,0xcb,0x3c,0x96,0xf5,0xd8,0xd9,0x0c,0x70,0x5d,0xb7,0xd1,0xe8,
  0xaa,0xe2,0x81,0xdf,0xd6,0xac,0xa5,0x68,0xad,0xdf,0x13,0xf3,0x92,0xba,0xfe,0x41,
  0xe2,0x24,0x8a,0x08,0x2d,0x98,0x12,0x80,0x2d,0x30,0x71,0x4f,0x0c,0x40,0x53,0xe3,
  0xfa,0x7a,0xe1,0x02,0x41,0x00,0xf8,0x20,0xf7,0x6b,0xcc,0xa6,0xc2,0xac,0xee,0xfd,
  0x2c,0xab,0xe8,0x3e,0xb4,0x10,0xb5,0x38,0xcb,0xea,0x88,0x1a,0x83,0xda,0xc8,0xf6,
  0x7d,0x99,0xba,0x82,0xb7,0xfd,0x75,0x72,0x6e,0x1d,0x9b,0xc8,0xe5,0xd3,0xf2,0xd0,
  0x39,0x1b,0x98,0xd2,0xcf,0xd0,0x1c,0xf1,0x77,0x95,0x3a,0xc9,0xc7,0xf6,0x04,0xf9,
  0xdd,0x65,0xde,0x68,0xb3,0x85,0x02,0x41,0x00,0xd6,0xfa,0xe6,0xfe,0x15,0x52,0x11,
  0xab,0x29,0xe5,0xe0,0x13,0x81,0x0e,0xe2,0x83,0xd0,0x85,0xf8,0x61,0x8a,0xaa,0x14,
  0xf0,0x5e,0x9f,0xee,0x39,0x29,0x7f,0x87,0x72,0x11,0x99,0x2b,0x6e,0xe4,0x9d,0x02,
  0xf4,0x09,0x18,0xcb,0x1d,0x1b,0x9f,0x78,0xf9,0x47,0xc8,0x94,0x11,0x7f,0xc2,0xd3,
  0x42,0x92,0x13,0x59,0x10,0x36,0x1e,0x0d,0x39,0x02,0x40,0x54,0xeb,0x0b,0x62,0x54,
  0x46,0xda,0x27,0x4f,0x1f,0x4c,0x53,0x85,0x07,0x38,0x9e,0xe3,0x36,0x8d,0xeb,0x35,
  0x2b,0x45,0xe6,0xf1,0x92,0x12,0x48,0x33,0x61,0x9b,0x50,0x5e,0x0a,0x8c,0x8f,0x0b,
  0x8a,0xc4,0xc5,0x35,0xbb,0x77,0x5f,0xef,0x7c,0x6e,0x6c,0xbe,0x0b,0x11,0xae,0xfb,
  0xd0,0xa4,0xe3,0x98,0xae,0x5e,0xcc,0x07,0x42,0xf3,0xf5,0x02,0x41,0x00,0xb8,0x60,
  0x01,0x62,0x62,0xc8,0xbb,0x2c,0x90,0x47,0x7d,0xd1,0x5c,0x7b,0x76,0x9b,0xd4,0x68,
  0x41,0xe8,0xee,0x33,0x06,0xb8,0x4c,0x7f,0x5f,0xa1,0x98,0xcc,0xae,0x4e,0x8f,0x77,
  0x4d,0x3f,0xd3,0x44,0x31,0xf7,0x38,0x22,0x12,0x79,0x49,0x6a,0x91,0x27,0x42,0x80,
  0x6d,0x35,0x40,0xdc,0xc7,0xb1,0x2d,0x74,0xf8,0x8a,0xa5,0xcf,0x4a,0x49,0x02,0x41,
  0x00,0xd2,0x8c,0xb8,0x41,0xc2,0xf1,0x27,0x4a,0xe5,0xb8,0x12,0x52,0x0c,0xcc,0x1c,
  0x0b,0x6e,0xee,0xf9,0xef,0x7c,0xf6,0x83,0x35,0x3c,0xd8,0xc8,0xfa,0xb6,0xba,0xf0,
  0x3a,0xca,0x28,0xc7,0xfb,0xe6,0x71,0xea,0x53,0xd5,0x68,0xf2,0xe0,0x0b,0xcf,0x64,
  0x78,0x31,0xa7,0xbb,0xc4,0x5c,0xd4,0x9e,0x4b,0xe3,0x39,0x2f,0x6f,0x50,0x35,0x3a,
  0x7c,
};

/**
 * And an associated self-signed certificate
-----BEGIN CERTIFICATE-----
MIIB9DCCAV2gAwIBAgIBADANBgkqhkiG9w0BAQUFADA3MQwwCgYDVQQDEwNzcnYx
EjAQBgNVBAsTCWxvYWQtdGVzdDETMBEGA1UEChMKc3Ryb25nU3dhbjAeFw0wODEy
MDgxODU4NDhaFw0xODEyMDYxODU4NDhaMDcxDDAKBgNVBAMTA3NydjESMBAGA1UE
CxMJbG9hZC10ZXN0MRMwEQYDVQQKEwpzdHJvbmdTd2FuMIGfMA0GCSqGSIb3DQEB
AQUAA4GNADCBiQKBgQDQXr7poAPYZLxmTCqR51STGRuk9Hc5SWtTcs6b2RzpnP8E
VRLxJEVxOKE9Mw6n7mD1pNrupCpnpGRdLAV5VznTPhSQ6k7ppJJrxosRYg0pHTZq
BUEC7nQFwAe10g8q0UnM1wa4lJzGxDH78d21cVweJgbkxAeyriS0jhNs7gO5nQID
AQABoxAwDjAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUAA4GBAF39Xedyk2wj
qOcaaZ7ypb8RDlLvS0uaJMVtLtIhtb2weMMlgdmOnKXEYrJL2/mbp14Fhe+XYME9
nZLAnmUnX8bQWCsQlajb7YGE8w6QDMwXUVgSXTMhRl+PRX2CMIUzU21h1EIx65Po
CwMLbJ7vQqwPHXRitDmNkEOK9H+vRnDf
-----END CERTIFICATE-----

 */
static char cert[] = {
  0x30,0x82,0x01,0xf4,0x30,0x82,0x01,0x5d,0xa0,0x03,0x02,0x01,0x02,0x02,0x01,0x00,
  0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x30,
  0x37,0x31,0x0c,0x30,0x0a,0x06,0x03,0x55,0x04,0x03,0x13,0x03,0x73,0x72,0x76,0x31,
  0x12,0x30,0x10,0x06,0x03,0x55,0x04,0x0b,0x13,0x09,0x6c,0x6f,0x61,0x64,0x2d,0x74,
  0x65,0x73,0x74,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0a,0x13,0x0a,0x73,0x74,
  0x72,0x6f,0x6e,0x67,0x53,0x77,0x61,0x6e,0x30,0x1e,0x17,0x0d,0x30,0x38,0x31,0x32,
  0x30,0x38,0x31,0x38,0x35,0x38,0x34,0x38,0x5a,0x17,0x0d,0x31,0x38,0x31,0x32,0x30,
  0x36,0x31,0x38,0x35,0x38,0x34,0x38,0x5a,0x30,0x37,0x31,0x0c,0x30,0x0a,0x06,0x03,
  0x55,0x04,0x03,0x13,0x03,0x73,0x72,0x76,0x31,0x12,0x30,0x10,0x06,0x03,0x55,0x04,
  0x0b,0x13,0x09,0x6c,0x6f,0x61,0x64,0x2d,0x74,0x65,0x73,0x74,0x31,0x13,0x30,0x11,
  0x06,0x03,0x55,0x04,0x0a,0x13,0x0a,0x73,0x74,0x72,0x6f,0x6e,0x67,0x53,0x77,0x61,
  0x6e,0x30,0x81,0x9f,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,
  0x01,0x05,0x00,0x03,0x81,0x8d,0x00,0x30,0x81,0x89,0x02,0x81,0x81,0x00,0xd0,0x5e,
  0xbe,0xe9,0xa0,0x03,0xd8,0x64,0xbc,0x66,0x4c,0x2a,0x91,0xe7,0x54,0x93,0x19,0x1b,
  0xa4,0xf4,0x77,0x39,0x49,0x6b,0x53,0x72,0xce,0x9b,0xd9,0x1c,0xe9,0x9c,0xff,0x04,
  0x55,0x12,0xf1,0x24,0x45,0x71,0x38,0xa1,0x3d,0x33,0x0e,0xa7,0xee,0x60,0xf5,0xa4,
  0xda,0xee,0xa4,0x2a,0x67,0xa4,0x64,0x5d,0x2c,0x05,0x79,0x57,0x39,0xd3,0x3e,0x14,
  0x90,0xea,0x4e,0xe9,0xa4,0x92,0x6b,0xc6,0x8b,0x11,0x62,0x0d,0x29,0x1d,0x36,0x6a,
  0x05,0x41,0x02,0xee,0x74,0x05,0xc0,0x07,0xb5,0xd2,0x0f,0x2a,0xd1,0x49,0xcc,0xd7,
  0x06,0xb8,0x94,0x9c,0xc6,0xc4,0x31,0xfb,0xf1,0xdd,0xb5,0x71,0x5c,0x1e,0x26,0x06,
  0xe4,0xc4,0x07,0xb2,0xae,0x24,0xb4,0x8e,0x13,0x6c,0xee,0x03,0xb9,0x9d,0x02,0x03,
  0x01,0x00,0x01,0xa3,0x10,0x30,0x0e,0x30,0x0c,0x06,0x03,0x55,0x1d,0x13,0x04,0x05,
  0x30,0x03,0x01,0x01,0xff,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,
  0x01,0x05,0x05,0x00,0x03,0x81,0x81,0x00,0x5d,0xfd,0x5d,0xe7,0x72,0x93,0x6c,0x23,
  0xa8,0xe7,0x1a,0x69,0x9e,0xf2,0xa5,0xbf,0x11,0x0e,0x52,0xef,0x4b,0x4b,0x9a,0x24,
  0xc5,0x6d,0x2e,0xd2,0x21,0xb5,0xbd,0xb0,0x78,0xc3,0x25,0x81,0xd9,0x8e,0x9c,0xa5,
  0xc4,0x62,0xb2,0x4b,0xdb,0xf9,0x9b,0xa7,0x5e,0x05,0x85,0xef,0x97,0x60,0xc1,0x3d,
  0x9d,0x92,0xc0,0x9e,0x65,0x27,0x5f,0xc6,0xd0,0x58,0x2b,0x10,0x95,0xa8,0xdb,0xed,
  0x81,0x84,0xf3,0x0e,0x90,0x0c,0xcc,0x17,0x51,0x58,0x12,0x5d,0x33,0x21,0x46,0x5f,
  0x8f,0x45,0x7d,0x82,0x30,0x85,0x33,0x53,0x6d,0x61,0xd4,0x42,0x31,0xeb,0x93,0xe8,
  0x0b,0x03,0x0b,0x6c,0x9e,0xef,0x42,0xac,0x0f,0x1d,0x74,0x62,0xb4,0x39,0x8d,0x90,
  0x43,0x8a,0xf4,0x7f,0xaf,0x46,0x70,0xdf,
};


/**
 * Default IKE preshared key
 */
static char *default_psk = "default-psk";

/**
 * Default EAP password for EAP
 */
static char *default_pwd = "default-pwd";


/**
 * Load the private key, hard-coded or from a file
 */
static private_key_t *load_issuer_key()
{
	char *path;

	path = lib->settings->get_str(lib->settings,
					"%s.plugins.load-tester.issuer_key", NULL, charon->name);
	if (!path)
	{
		return lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
					BUILD_BLOB_ASN1_DER, chunk_create(private, sizeof(private)),
					BUILD_END);
	}
	DBG1(DBG_CFG, "loading load-tester private key from '%s'", path);
	return lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
					BUILD_FROM_FILE, path, BUILD_END);
}

/**
 * Load the issuing certificate, hard-coded or from a file
 */
static certificate_t *load_issuer_cert()
{
	char *path;

	path = lib->settings->get_str(lib->settings,
					"%s.plugins.load-tester.issuer_cert", NULL, charon->name);
	if (!path)
	{
		return lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
					BUILD_BLOB_ASN1_DER, chunk_create(cert, sizeof(cert)),
					BUILD_X509_FLAG, X509_CA,
					BUILD_END);
	}
	DBG1(DBG_CFG, "loading load-tester issuer cert from '%s'", path);
	return lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
					BUILD_FROM_FILE, path, BUILD_END);
}

/**
 * Load (intermediate) CA certificates, hard-coded or from a file
 */
static void load_ca_certs(private_load_tester_creds_t *this)
{
	enumerator_t *enumerator;
	certificate_t *cert;
	struct stat st;
	char *path;

	path = lib->settings->get_str(lib->settings,
						"%s.plugins.load-tester.ca_dir", NULL, charon->name);
	if (path)
	{
		enumerator = enumerator_create_directory(path);
		if (enumerator)
		{
			while (enumerator->enumerate(enumerator, NULL, &path, &st))
			{
				if (S_ISREG(st.st_mode))
				{
					DBG1(DBG_CFG, "loading load-tester CA cert from '%s'", path);
					cert = lib->creds->create(lib->creds,
											CRED_CERTIFICATE, CERT_X509,
											BUILD_FROM_FILE, path, BUILD_END);
					if (cert)
					{
						this->cas->insert_last(this->cas, cert);
					}
				}
			}
			enumerator->destroy(enumerator);
		}
	}
}

METHOD(credential_set_t, create_private_enumerator, enumerator_t*,
	private_load_tester_creds_t *this, key_type_t type, identification_t *id)
{
	if (this->private == NULL)
	{
		return NULL;
	}
	if (type != KEY_ANY && type != KEY_RSA)
	{
		return NULL;
	}
	if (id)
	{
		if (!this->private->has_fingerprint(this->private, id->get_encoding(id)))
		{
			return NULL;
		}
	}
	return enumerator_create_single(this->private, NULL);
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_load_tester_creds_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	enumerator_t *enumerator;
	certificate_t *peer_cert, *ca_cert;
	public_key_t *peer_key, *ca_key;
	identification_t *dn = NULL;
	linked_list_t *sans;
	char buf[128];
	u_int32_t serial;
	time_t now;

	if (this->ca == NULL)
	{
		return NULL;
	}
	if (cert != CERT_ANY && cert != CERT_X509)
	{
		return NULL;
	}
	if (key != KEY_ANY && key != KEY_RSA)
	{
		return NULL;
	}
	if (!id)
	{
		return this->cas->create_enumerator(this->cas);
	}
	ca_key = this->ca->get_public_key(this->ca);
	if (ca_key)
	{
		if (ca_key->has_fingerprint(ca_key, id->get_encoding(id)))
		{
			ca_key->destroy(ca_key);
			return enumerator_create_single(this->ca, NULL);
		}
		ca_key->destroy(ca_key);
	}
	enumerator = this->cas->create_enumerator(this->cas);
	while (enumerator->enumerate(enumerator, &ca_cert))
	{
		if (ca_cert->has_subject(ca_cert, id))
		{
			enumerator->destroy(enumerator);
			return enumerator_create_single(ca_cert, NULL);
		}
	}
	enumerator->destroy(enumerator);

	if (!trusted)
	{
		/* peer certificate, generate on demand */
		serial = htonl(++this->serial);
		now = time(NULL);
		sans = linked_list_create();

		switch (id->get_type(id))
		{
			case ID_DER_ASN1_DN:
				break;
			case ID_FQDN:
			case ID_RFC822_ADDR:
			case ID_IPV4_ADDR:
			case ID_IPV6_ADDR:
				/* encode as subjectAltName, construct a sane DN */
				sans->insert_last(sans, id);
				snprintf(buf, sizeof(buf), "CN=%Y", id);
				dn = identification_create_from_string(buf);
				break;
			default:
				sans->destroy(sans);
				return NULL;
		}
		peer_key = this->private->get_public_key(this->private);
		peer_cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
									BUILD_SIGNING_KEY, this->private,
									BUILD_SIGNING_CERT, this->ca,
									BUILD_DIGEST_ALG, this->digest,
									BUILD_PUBLIC_KEY, peer_key,
									BUILD_SUBJECT, dn ?: id,
									BUILD_SUBJECT_ALTNAMES, sans,
									BUILD_NOT_BEFORE_TIME, now - 60 * 60 * 24,
									BUILD_NOT_AFTER_TIME, now + 60 * 60 * 24,
									BUILD_SERIAL, chunk_from_thing(serial),
									BUILD_END);
		peer_key->destroy(peer_key);
		sans->destroy(sans);
		DESTROY_IF(dn);
		if (peer_cert)
		{
			return enumerator_create_single(peer_cert, (void*)peer_cert->destroy);
		}
	}
	return NULL;
}

/**
 * Filter function for shared keys, returning ID matches
 */
static bool shared_filter(void *null, shared_key_t **in, shared_key_t **out,
				void **un1, id_match_t *me, void **un2, id_match_t *other)
{
	*out = *in;
	if (me)
	{
		*me = ID_MATCH_ANY;
	}
	if (other)
	{
		*other = ID_MATCH_ANY;
	}
	return TRUE;
}

METHOD(credential_set_t, create_shared_enumerator, enumerator_t*,
	private_load_tester_creds_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	shared_key_t *shared;

	switch (type)
	{
		case SHARED_IKE:
			shared = this->psk;
			break;
		case SHARED_EAP:
			shared = this->pwd;
			break;
		default:
			return NULL;
	}
	return enumerator_create_filter(enumerator_create_single(shared, NULL),
									(void*)shared_filter, NULL, NULL);
}

METHOD(load_tester_creds_t, destroy, void,
	private_load_tester_creds_t *this)
{
	this->cas->destroy_offset(this->cas, offsetof(certificate_t, destroy));
	DESTROY_IF(this->private);
	DESTROY_IF(this->ca);
	this->psk->destroy(this->psk);
	this->pwd->destroy(this->pwd);
	free(this);
}

load_tester_creds_t *load_tester_creds_create()
{
	private_load_tester_creds_t *this;
	char *pwd, *psk, *digest;

	psk = lib->settings->get_str(lib->settings,
			"%s.plugins.load-tester.preshared_key", default_psk, charon->name);
	pwd = lib->settings->get_str(lib->settings,
			"%s.plugins.load-tester.eap_password", default_pwd, charon->name);
	digest = lib->settings->get_str(lib->settings,
			"%s.plugins.load-tester.digest", "sha1", charon->name);

	INIT(this,
		.public = {
			.credential_set = {
				.create_shared_enumerator = _create_shared_enumerator,
				.create_private_enumerator = _create_private_enumerator,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
		.private = load_issuer_key(),
		.ca = load_issuer_cert(),
		.cas = linked_list_create(),
		.digest = enum_from_name(hash_algorithm_short_names, digest),
		.psk = shared_key_create(SHARED_IKE,
								 chunk_clone(chunk_create(psk, strlen(psk)))),
		.pwd = shared_key_create(SHARED_EAP,
								 chunk_clone(chunk_create(pwd, strlen(pwd)))),
	);

	if (this->ca)
	{
		this->cas->insert_last(this->cas, this->ca->get_ref(this->ca));
	}

	if (this->digest == -1)
	{
		DBG1(DBG_CFG, "invalid load-tester digest: '%s', using sha1", digest);
		this->digest = HASH_SHA1;
	}

	load_ca_certs(this);

	return &this->public;
}

