#include <linux/fs.h>
#include <asm/uaccess.h>
#include "fwall.h"



#define MY_MACIG 'G'
#define READ_IOCTL _IOR(MY_MACIG, 0, int)
#define WRITE_IOCTL _IOW(MY_MACIG, 1, int)

#define __KERN__
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/stat.h>

static struct nf_hook_ops netfilter_ops;
static unsigned char *remote_ip_add = "\x0A\x00\x00\x02";
//static unsigned char *remote_ip_add = "\x13\x00\x06\x0B";
static unsigned char *server_ip_add = "\xC0\xA8\x12\x04";
unsigned char *telnet_port = "\x00\x17";
unsigned char *http_port = "\x00\x50";
unsigned char *type = "\x00";
static int rules_arr[3] ;
static int arg_count;
static int rule1 =0;                                                                                                                                           
static int rule2=0;                                                                                                                                            
static int rule3=0;                                                                                                                                            

 
static int major; 
static char msg[200];

struct sk_buff *sock_buff;
struct iphdr* iph;
struct tcphdr *tcph;
struct icmphdr *icmph;


unsigned int main_hook(unsigned int hooknum,

                  struct sk_buff **skb,

                  const struct net_device *in,

                  const struct net_device *out,

                  int (*okfn)(struct sk_buff*) )
{



 

    iph =(struct iphdr *)(skb_network_header(skb));

    tcph =(struct tcphdr *)((__u32*)iph +iph->ihl);
    if(!skb) {

        return NF_ACCEPT;
    }
   if (rule1== 1) {


//        printk(KERN_ALERT "Rule 1 is selected");

        if(iph->protocol == 1) {

              icmph = (struct icmphdr *)(skb_transport_header(skb));

              if ((iph->saddr==*(unsigned int *)remote_ip_add ) & (iph->daddr==*(unsigned int *)server_ip_add))  {

                        printk(KERN_INFO "Allowed: Cause: ICMP packets, Interface:eth2, Dest: 10.0.0.1");

                        return NF_ACCEPT;

              } else if ((iph->saddr==*(unsigned int *)remote_ip_add) & (icmph->type==*type)) {

                        printk(KERN_INFO "Allowed: Cause: ICMP Echo reply, Interface:eth2, Dest: Any host");

                        return NF_ACCEPT;

              } else if (iph->saddr==*(unsigned int *)remote_ip_add) {

                        printk(KERN_INFO "Dropped: Cause: ICMP packets, Interface:eth2, Dest: Any host other than webserver");

                        return NF_DROP;
              } else {

                        return NF_ACCEPT;

              }

        }
   }
  /* else {
        printk(KERN_ALERT "Rule 1 NOT selected");

   }
*/


/* Block ssh attempts from outside */

   if (rule2 == 1) {

       printk(KERN_ALERT "Rule 2 is selected\n");

        if(iph->protocol == 6 || iph->protocol == 17) {
                if ((iph->daddr==*(unsigned int *)remote_ip_add) & (tcph->dest==*(unsigned short *)telnet_port)) {

                        printk(KERN_INFO "Dropped: Cause: Telnet, Interface: eth2, Dest: Any host, Dest port: 23");
                        return NF_DROP;
                }

        }

   } 
        /*else {

        printk(KERN_ALERT "Rule2 NOT selected");
   }*/

/*Block http access from remote client*/

   if (rule3 == 1) {

     //    printk(KERN_ALERT "Rule 3 is selected");

        if (tcph->dest==*(unsigned short *)http_port) {

                /* Block All HTTP accesses except access to webserver. */

                if ((iph->saddr==*(unsigned int *)remote_ip_add)) {

                        if (iph->daddr==*(unsigned int *)server_ip_add) {

                                printk(KERN_INFO "Allowed: Cause: HTTP, Interface: eth2, Dest: 192.168.18.4, Dest port: 80");

                                return NF_ACCEPT;
                        } else {

                                printk(KERN_INFO "Dropped: cause: HTTP, Interface: eth2, Dest: Any host other than webserver, Dest Port: 80");
                                return NF_DROP;
                        }

                }

        }

   } 
        /*else {

        printk(KERN_ALERT "Rule3 NOT selected");
   }*/

   return NF_ACCEPT;

}






static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
        return simple_read_from_buffer(buffer, length, offset, msg, 200);
}


static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
        if (len > 199)
                return -EINVAL;
        copy_from_user(msg, buff, len);
        msg[len] = '\0';
        kstrtoint(msg,10,&rule1);
        printk(KERN_ALERT "CHG %d\n",  rule1);
        return len;
}
char buf[200];
mystruct ob;
long device_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
        int len = 200;
        switch(cmd) {
        case READ_IOCTL:
                copy_to_user((char *)arg, buf, 200);
                break;

        case WRITE_IOCTL:
                //copy_from_user(buf, (char *)arg, len);
                copy_from_user(&ob, (char *)arg, sizeof(ob));
                //kstrtoint(buf,10,&rule1);
                rule1=((mystruct)ob).r1;
                rule2=((mystruct)ob).r2;
                rule3=((mystruct)ob).r3;
                break;

        default:
                return -ENOTTY;
        }
        return len;

}
static struct file_operations fops = {
        .read = device_read, 
        .write = device_write,
        .unlocked_ioctl = device_ioctl, // ioctl 
};

static int __init cdevexample_module_init(void)
{

        netfilter_ops.hook              =       main_hook;

        netfilter_ops.pf                =       PF_INET;

        netfilter_ops.hooknum           =       NF_INET_PRE_ROUTING;

        netfilter_ops.priority          =       NF_IP_PRI_FIRST;

        nf_register_hook(&netfilter_ops);


        major = register_chrdev(0, "my_device", &fops);
        if (major < 0) {
                printk ("Registering the character device failed with %d\n", major);
                return major;
        }
        printk("cdev example: assigned major: %d\n", major);
        printk("create node with mknod /dev/cdev_example c %d 0\n", major);
        return 0;
}

static void __exit cdevexample_module_exit(void)
{
        nf_unregister_hook(&netfilter_ops);
        unregister_chrdev(major, "my_device");
}  

module_init(cdevexample_module_init);
module_exit(cdevexample_module_exit);
MODULE_LICENSE("GPL");
