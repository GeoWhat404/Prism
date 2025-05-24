#include <cpuid.h>
#include <stdint.h>
#include <util/logger.h>

#define cpuid(in, a, b, c, d)   \
    __asm__ volatile("cpuid": "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(in));


int handle_intel(void);
int handle_amd(void);

char *intel_models[] = {
    "Brand ID Not Supported.", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Intel(R) Pentium(R) III Xeon(R) processor", 
	"Intel(R) Pentium(R) III processor", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) III processor-M", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Pentium(R) 4 processor", 
	"Intel(R) Celeron(R) processor", 
	"Intel(R) Xeon(R) Processor", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Mobile Intel(R) Pentium(R) 4 processor-M", 
	"Mobile Intel(R) Pentium(R) Celeron(R) processor", 
	"Reserved", 
	"Mobile Genuine Intel(R) processor", 
	"Intel(R) Celeron(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor", 
	"Intel(R) Celeron(R) processor", 
	"Mobile Geniune Intel(R) processor", 
	"Intel(R) Pentium(R) M processor", 
	"Mobile Intel(R) Celeron(R) processor"
};

char *intel_other[] = {
    "Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Celeron(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor MP", 
	"Reserved", 
	"Reserved", 
	"Intel(R) Xeon(R) processor", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved", 
	"Reserved",
	"Reserved", 
	"Reserved"
};

int detect_cpu() {
    uint32_t ebx, unused;

    cpuid(0, unused, ebx, unused, unused);

    kinfo("--- < cpu detection > ---");
    switch (ebx) {
        case signature_INTEL_ebx:
            handle_intel();
            break;
        case signature_AMD_ebx:
            handle_amd();
            break;
        default:
            kinfo("Unknown x86 CPU detected");
            break;
    }
    kinfo("--- < cpu detection > ---");
    return 0;
}

void printregs(int eax, int ebx, int ecx, int edx) {
	int j;
	char string[17];
	string[16] = '\0';
	for(j = 0; j < 4; j++) {
		string[j] = eax >> (8 * j);
		string[j + 4] = ebx >> (8 * j);
		string[j + 8] = ecx >> (8 * j);
		string[j + 12] = edx >> (8 * j);
	}
	kinfo("%s", string);
}

char *intel_get_type(int type) {
    switch (type) {
        case 0: return "Original OEM";
        case 1: return "Overdrive";
        case 2: return "Dual-capable";
        case 3: return "Reserved";
    }
    return "Unknown";
}

char *intel_get_family(int family) {
    switch (family) {
        case 3: return "i386";
        case 4: return "i486";
        case 5: return "Pentium";
        case 6: return "Pentium Pro";
        case 15: return "Pentium IV";
    }
    return "Unknown";
}

char *intel_get_model(int family, int model) {
    switch (family) {
        case 3: return "???";
        case 4:
            switch (model) {
                case 0:
                case 1: return "DX";
                case 2: return "SX";
                case 3: return "487/DX2";
                case 4: return "SL";
                case 5: return "SX2";
                case 7: return "Write-back enhanced DX2";
                case 8: return "DX4";
            }
            break;
        case 5:
            switch (model) {
                case 1: return "60/66";
                case 2: return "75-200";
                case 3: return "for 486 system";
                case 4: return "MMX";
            }
            break;
        case 6:
            switch (model) {
                case 1: return "Pentium Pro";
                case 3: return "Pentium II Model 3";
                case 5: return "Pentium II Model 5/Xeon/Celeron";
                case 6: return "Celeron";
                case 7: return "Pentium III/Pentium III Xeon - external L2 cache";
                case 8: return "Pentium III/Pentium III Xeon - internal L2 cache";
            }
            break;
        case 15:
            return "???";
    }
    return "???";
}

int handle_intel() {
    kinfo("Intel specific features:");
    uint32_t eax, ebx, ecx, edx, max_eax, sig, unused;
    int model, family, type, brand, stepping, reserved;
    int extended_family = -1;
    
    cpuid(1, eax, ebx, unused, unused);
    model = (eax >> 4) & 0xF;
    family = (eax >> 8) & 0xF;
    type = (eax >> 12) & 0x3;
    brand = ebx & 0xFF;
    stepping = eax & 0xF;
    reserved = eax >> 14;
    sig = eax;

    kinfo("\t Type %d - %s", type, intel_get_type(type));
    kinfo("Family %d - %s", family, intel_get_family(family));

    if (family == 15) {
        extended_family = (eax >> 20) & 0xFF;
        kinfo("Extended family %d", extended_family);
    }
    kinfo("Model %d - %s", model, intel_get_model(family, model));

	cpuid(0x80000000, max_eax, unused, unused, unused);

    if (max_eax >= 0x80000004) {
        kinfo("Brand: ");
        if (max_eax >= 0x80000002) {
            cpuid(0x80000002, eax, ebx, ecx, edx);
            printregs(eax, ebx, ecx, edx);
        }
        if (max_eax >= 0x80000003) {
            cpuid(0x80000003, eax, ebx, ecx, edx);
            printregs(eax, ebx, ecx, edx);
        }
        if (max_eax >= 0x80000004) {
            cpuid(0x80000004, eax, ebx, ecx, edx);
            printregs(eax, ebx, ecx, edx);
        }
        kinfo("");
    } else if (brand > 0) {
        kinfo("Brand %d - ", brand);
        if (brand < 0x18) {
            if (sig == 0x000006B1 || sig == 0x00000F13) {
                kinfo("%s", intel_other[brand]);
            } else {
                kinfo("%s", intel_models[brand]);
            }
        } else {
            kinfo("Reserved");
        }
    }
    kinfo("Stepping: %d Reserved: %d", stepping, reserved);
    return 0;
}

int handle_amd() {
    kinfo("AMD Specific Features:");
    uint32_t extended, eax, ebx, ecx, edx, unused;
    int family, model, stepping, reserved;

    cpuid(1, eax, unused, unused, unused);
    model = (eax >> 4) & 0xF;
    family = (eax >> 8) & 0xF;
    stepping = eax & 0xF;
    reserved = eax >> 12;

    kinfo("\t Family: %d Model %d [", family, model);
    switch(family) {
        case 4:
            kinfo("486 Model %d", model);
            break;
        case 5:
            switch(model) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 6:
                case 7:
                    kinfo("K6 Model %d", model);
                    break;
                case 8:
                    kinfo("K6-2 Model 8");
                    break;
                case 9:
                    kinfo("K6-III Model 9");
                    break;
                default:
                    kinfo("K5/K6 Model %d", model);
                    break;
            }
            break;
        case 6:
            switch(model) {
                case 1:
                case 2:
                case 4:
                    kinfo("Athlon Model %d", model);
                    break;
                case 3:
                    kinfo("Duron Model 3");
                    break;
                case 6:
                    kinfo("Athlon MP/Mobile Athlon Model 6");
                    break;
                case 7:
                    kinfo("Mobile Duron Model 7");
                    break;
                default:
                    kinfo("Duron/Athlon Model %d", model);
                    break;
            }
            break;
    }
    kinfo("]");

    cpuid(0x80000000, extended, unused, unused, unused);
    if (extended == 0)
        return 0;

    if (extended >= 0x80000002) {
        uint32_t i;
        kinfo("Detected Processor Name: ");
        for (i = 0x80000002; i < 0x80000004; i++) {
            cpuid(i, eax, ebx, ecx, edx);
            printregs(eax, ebx, ecx, edx);
        }
    }

    if (extended >= 0x80000007) {
        cpuid(0x80000007, unused, unused, unused, edx);
        if (edx & 1) {
            kinfo("Temperature Sensing Diode Detected!");
        }
    }
    kinfo("Stepping: %d Reserved: %d", stepping, reserved);
    return 0;
}
