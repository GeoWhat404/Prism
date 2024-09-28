#include <cpuid.h>
#include <stdio.h>

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

int detect_cpu(void) {
    uint32_t ebx, unused;

    cpuid(0, unused, ebx, unused, unused);

    switch (ebx) {
        case signature_INTEL_ebx:
            return handle_intel();
        case signature_AMD_ebx:
            return handle_amd();
        default:
            printf("Unknown x86 CPU detected\n");
            break;
    }
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
	printf("%s", string);
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

int handle_intel(void) {
    printf("Intel specific features:\n");
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

    printf("\t Type %d - %s\n", type, intel_get_type(type));
    printf("Family %d - %s\n", family, intel_get_family(family));

    if (family == 15) {
        extended_family = (eax >> 20) & 0xFF;
        printf("Extended family %d\n", extended_family);
    }
    printf("Model %d - %s\n", model, intel_get_model(family, model));

	cpuid(0x80000000, max_eax, unused, unused, unused);

    if (max_eax >= 0x80000004) {
        printf("Brand: ");
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
        printf("\n");
    } else if (brand > 0) {
        printf("Brand %d - ", brand);
        if (brand < 0x18) {
            if (sig == 0x000006B1 || sig == 0x00000F13) {
                printf("%s\n", intel_other[brand]);
            } else {
                printf("%s\n", intel_models[brand]);
            }
        } else {
            printf("Reserved\n");
        }
    }
    printf("Stepping: %d Reserved: %d\n", stepping, reserved);
    return 0;
}

int handle_amd(void) {
    printf("AMD Specific Features:\n");
    uint32_t extended, eax, ebx, ecx, edx, unused;
    int family, model, stepping, reserved;

    cpuid(1, eax, unused, unused, unused);
    model = (eax >> 4) & 0xF;
    family = (eax >> 8) & 0xF;
    stepping = eax & 0xF;
    reserved = eax >> 12;

    printf("\t Family: %d Model %d [", family, model);
    switch(family) {
        case 4:
            printf("486 Model %d", model);
            break;
        case 5:
            switch(model) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 6:
                case 7:
                    printf("K6 Model %d", model);
                    break;
                case 8:
                    printf("K6-2 Model 8");
                    break;
                case 9:
                    printf("K6-III Model 9");
                    break;
                default:
                    printf("K5/K6 Model %d", model);
                    break;
            }
            break;
        case 6:
            switch(model) {
                case 1:
                case 2:
                case 4:
                    printf("Athlon Model %d", model);
                    break;
                case 3:
                    printf("Duron Model 3");
                    break;
                case 6:
                    printf("Athlon MP/Mobile Athlon Model 6");
                    break;
                case 7:
                    printf("Mobile Duron Model 7");
                    break;
                default:
                    printf("Duron/Athlon Model %d", model);
                    break;
            }
            break;
    }
    printf("]\n");

    cpuid(0x80000000, extended, unused, unused, unused);
    if (extended == 0)
        return 0;

    if (extended >= 0x80000002) {
        uint32_t i;
        printf("Detected Processor Name: ");
        for (i = 0x80000002; i < 0x80000004; i++) {
            cpuid(i, eax, ebx, ecx, edx);
            printregs(eax, ebx, ecx, edx);
        }
        printf("\n");
    }

    if (extended >= 0x80000007) {
        cpuid(0x80000007, unused, unused, unused, edx);
        if (edx & 1) {
            printf("Temperature Sensing Diode Detected!\n");
        }
    }
    printf("Stepping: %d Reserved: %d\n", stepping, reserved);
    return 0;
}
