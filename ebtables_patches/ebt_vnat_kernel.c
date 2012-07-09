/*
 *  ebt_vnat
 *
 *	Authors: Ashwin Kashyap <cybernytrix@yahoo.com>
 *
 * This module rewrites ether frames on the bridge to include
 * the 802.1q header. 

 * 
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_nat.h>
#include <linux/module.h>
#include <linux/if_vlan.h>

#define VNAT_DEBUG


// stolen from 8021q/vlan_dev.c and ebt_snat.c
static int ebt_target_vnat(struct sk_buff **pskb, unsigned int hooknr,
   const struct net_device *in, const struct net_device *out,
   const void *data, unsigned int datalen)

{
	struct ebt_nat_info *info = (struct ebt_nat_info *) data;
	struct vlan_ethhdr *veth = NULL;
	unsigned short veth_TCI;

	printk("vid tagged %hd\n", info->vid);
	
	if (info->vid == (uint16_t)-1) {

		veth = (struct vlan_ethhdr *)(*pskb)->mac.raw;
		// TODO: strip tag
		printk("vnat_strip_tag: %x, %x, %x\n",
			(*pskb)->mac.raw,
			(*pskb)->nh.raw,
			(*pskb)->data);
		if (veth->h_vlan_proto == __constant_htons(ETH_P_8021Q)) {

			uint16_t mark = ntohs(veth->h_vlan_TCI);
			/* copy the information onto nfmark */
			if ((*pskb)->nfmark != mark) {
				(*pskb)->nfmark = mark;
				(*pskb)->nfcache |= NFC_ALTERED;
			}


			memmove((*pskb)->mac.raw + VLAN_HLEN, (*pskb)->mac.raw, 12);
			(*pskb)->mac.raw += VLAN_HLEN;
			(*pskb)->nh.raw += VLAN_HLEN;
			(*pskb)->data += VLAN_HLEN;
		}
	} else {

		/* Make sure we have enough room */

		if (skb_headroom(*pskb) < VLAN_HLEN) {
			struct sk_buff *sk_tmp = *pskb;
			printk("%s: In if skb_headroom\n", __FUNCTION__);
			*pskb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
			kfree_skb(sk_tmp);

			if (*pskb == NULL) {
				return EBT_DROP;
			}
		}
		// This actually pushes skb->data, returns new of skb->data
		// TODO: make sure there is sufficient headroom to decrement mac.raw
		skb_push(*pskb, VLAN_HLEN);

		(*pskb)->mac.raw -= VLAN_HLEN;
		(*pskb)->nh.raw -= VLAN_HLEN;
		veth = (struct vlan_ethhdr *) (*pskb)->mac.raw;

		/* Move the mac addresses to the beginning of the new header. */
		printk("vnat: proto_dbg: %x, %x\n", veth->h_vlan_encapsulated_proto, veth->h_vlan_proto);

		memmove((*pskb)->mac.raw, (*pskb)->mac.raw + VLAN_HLEN, 12);

		/* first, the ethernet type */
		veth->h_vlan_proto = __constant_htons(ETH_P_8021Q);

		/* Now, construct the second two bytes. This field looks something

		 * like:
		 * usr_priority: 3 bits	 (high bits)
		 * CFI		 1 bit
		 * VLAN ID	 12 bits (low bits)
		 */
		veth_TCI = info->vid;
		veth_TCI |= info->qos << 13;
		veth->h_vlan_TCI = htons(veth_TCI);


	}
#ifdef VNAT_DEBUG 
	printk("%s: about to send skb: %p to dev: %s\n",
		__FUNCTION__, *pskb, (*pskb)->dev->name);
	printk("dst: %2hx.%2hx.%2hx.%2hx.%2hx.%2hx "
			"src: %2hx.%2hx.%2hx.%2hx.%2hx.%2hx "

			"misc: %4hx %4hx %4hx\n",
	       veth->h_dest[0], veth->h_dest[1], veth->h_dest[2], veth->h_dest[3], veth->h_dest[4], veth->h_dest[5],
	       veth->h_source[0], veth->h_source[1], veth->h_source[2], veth->h_source[3], veth->h_source[4], veth->h_source[5],

	       veth->h_vlan_proto, veth->h_vlan_TCI, veth->h_vlan_encapsulated_proto);
#endif
	//return EBT_DROP; // for DEBUGGING
	return info->target;
}

static int ebt_target_vnat_check(const char *tablename, unsigned int hookmask,

   const struct ebt_entry *e, void *data, unsigned int datalen)
{
	struct ebt_nat_info *info = (struct ebt_nat_info *) data;

	if (datalen != EBT_ALIGN(sizeof(struct ebt_nat_info))) {
		printk("ebt_vnat: Bad Align\n");

		return -EINVAL;
	}
	if (BASE_CHAIN && info->target == EBT_RETURN) {
		printk("ebt_vnat: Bad Align\n");
		return -EINVAL;
	}
	CLEAR_BASE_CHAIN_BIT;
	if ( (strcmp(tablename, "nat") ||

	   (hookmask & ~((1 << NF_BR_PRE_ROUTING) | (1 << NF_BR_LOCAL_OUT)))) &&
	   (strcmp(tablename, "broute") || hookmask & ~(1 << NF_BR_BROUTING)) ) {
		printk("Wrong chain for vnat\n");

		return -EINVAL;
	}
	if (INVALID_TARGET) {
		printk("ebt_vnat: Bad target\n");
		return -EINVAL;
	}
	printk("ebt_vnat: check ok, added with vlan id: %d, qos: %d\n", info->vid, info->qos);

	return 0;
}



static struct ebt_target vnat =
{
	{NULL, NULL}, EBT_VNAT_TARGET, ebt_target_vnat, ebt_target_vnat_check,
	NULL, THIS_MODULE
};

static int __init init(void)
{
	printk("ebt_vnat loaded ok\n");

	return ebt_register_target(&vnat);
}

static void __exit fini(void)
{
	ebt_unregister_target(&vnat);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;


MODULE_LICENSE("GPL");
