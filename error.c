#include "everything.h"

#ifdef FREEBSD
static const char *strings[] = {"EPERM: Operation not permitted.", "ENOENT: No such file or directory.", "ESRCH: No such process.", "EINTR: Interrupted system call.", "EIO: Input/output error.", "ENXIO: Device not configured.", "E2BIG: Argument list too long.", "ENOEXEC: Exec format error.", "EBADF: Bad file descriptor.", "ECHILD: No child processes.", "EDEADLK: Resource deadlock avoided.", "ENOMEM: Cannot allocate memory.", "EACCES: Permission denied.", "EFAULT: Bad address.", "ENOTBLK: Block device required.", "EBUSY: Device busy.", "EEXIST: File exists.", "EXDEV: Cross-device link.", "ENODEV: Operation not supported by device.", "ENOTDIR: Not a directory.", "EISDIR: Is a directory.", "EINVAL: Invalid argument.", "ENFILE: Too many open files in system.", "EMFILE: Too many open files.", "ENOTTY: Inappropriate ioctl for device.", "ETXTBSY: Text file busy.", "EFBIG: File too large.", "ENOSPC: No space left on device.", "ESPIPE: Illegal seek.", "EROFS: Read-only filesystem.", "EMLINK: Too many links.", "EPIPE: Broken pipe.", "EDOM: Numerical argument out of domain.", "ERANGE: Result too large.", "EAGAIN: Resource temporarily unavailable.", "EINPROGRESS: Operation now in progress.", "EALREADY: Operation already in progress.", "ENOTSOCK: Socket operation on non-socket.", "EDESTADDRREQ: Destination address required.", "EMSGSIZE: Message too long.", "EPROTOTYPE: Protocol wrong type for socket.", "ENOPROTOOPT: Protocol not available.", "EPROTONOSUPPORT: Protocol not supported.", "ESOCKTNOSUPPORT: Socket type not supported.", "EOPNOTSUPP: Operation not supported.", "EPFNOSUPPORT: Protocol family not supported.", "EAFNOSUPPORT: Address family not supported by protocol family.", "EADDRINUSE: Address already in use.", "EADDRNOTAVAIL: Can't assign requested address.", "ENETDOWN: Network is down.", "ENETUNREACH: Network is unreachable.", "ENETRESET: Network dropped connection on reset.", "ECONNABORTED: Software caused connection abort.", "ECONNRESET: Connection reset by peer.", "ENOBUFS: No buffer space available.", "EISCONN: Socket is already connected.", "ENOTCONN: Socket is not connected.", "ESHUTDOWN: Can't send after socket shutdown.", "ETOOMANYREFS: Too many references: can't splice.", "ETIMEDOUT: Operation timed out.", "ECONNREFUSED: Connection refused.", "ELOOP: Too many levels of symbolic links.", "ENAMETOOLONG: File name too long.", "EHOSTDOWN: Host is down.", "EHOSTUNREACH: No route to host.", "ENOTEMPTY: Directory not empty.", "EPROCLIM: Too many processes.", "EUSERS: Too many users.", "EDQUOT: Disc quota exceeded.", "ESTALE: Stale NFS file handle.", "EREMOTE: Too many levels of remote in path.", "EBADRPC: RPC struct is bad.", "ERPCMISMATCH: RPC version wrong.", "EPROGUNAVAIL: RPC prog. not avail.", "EPROGMISMATCH: Program version wrong.", "EPROCUNAVAIL: Bad procedure for program.", "ENOLCK: No locks available.", "ENOSYS: Function not implemented.", "EFTYPE: Inappropriate file type or format.", "EAUTH: Authentication error.", "ENEEDAUTH: Need authenticator.", "EIDRM: Identifier removed.", "ENOMSG: No message of desired type.", "EOVERFLOW: Value too large to be stored in data type.", "ECANCELED: Operation canceled.", "EILSEQ: Illegal byte sequence.", "ENOATTR: Attribute not found.", "EDOOFUS: Programming error.", "EBADMSG: Bad message.", "EMULTIHOP: Multihop attempted.", "ENOLINK: Link has been severed.", "EPROTO: Protocol error.", "ENOTCAPABLE: Capabilities insufficient.", "ECAPMODE: Not permitted in capability mode."};
#endif

#ifdef LINUX
static const char *strings[] = {"EPERM: Operation not permitted.", "ENOENT: No such file or directory.", "ESRCH: No such process.", "EINTR: Interrupted system call.", "EIO: I/O error.", "ENXIO: No such device or address.", "E2BIG: Argument list too long.", "ENOEXEC: Exec format error.", "EBADF: Bad file number.", "ECHILD: No child processes.", "EAGAIN: Try again.", "ENOMEM: Out of memory.", "EACCES: Permission denied.", "EFAULT: Bad address.", "ENOTBLK: Block device required.", "EBUSY: Device or resource busy.", "EEXIST: File exists.", "EXDEV: Cross-device link.", "ENODEV: No such device.", "ENOTDIR: Not a directory.", "EISDIR: Is a directory.", "EINVAL: Invalid argument.", "ENFILE: File table overflow.", "EMFILE: Too many open files.", "ENOTTY: Not a typewriter.", "ETXTBSY: Text file busy.", "EFBIG: File too large.", "ENOSPC: No space left on device.", "ESPIPE: Illegal seek.", "EROFS: Read-only file system.", "EMLINK: Too many links.", "EPIPE: Broken pipe.", "EDOM: Math argument out of domain of func.", "ERANGE: Math result not representable.", "EDEADLK: Resource deadlock would occur.", "ENAMETOOLONG: File name too long.", "ENOLCK: No record locks available.", "ENOSYS: Function not implemented.", "ENOTEMPTY: Directory not empty.", "ELOOP: Too many symbolic links encountered.", "EWOULDBLOCK: Operation would block.", "ENOMSG: No message of desired type.", "EIDRM: Identifier removed.", "ECHRNG: Channel number out of range.", "EL2NSYNC: Level 2 not synchronized.", "EL3HLT: Level 3 halted.", "EL3RST: Level 3 reset.", "ELNRNG: Link number out of range.", "EUNATCH: Protocol driver not attached.", "ENOCSI: No CSI structure available.", "EL2HLT: Level 2 halted.", "EBADE: Invalid exchange.", "EBADR: Invalid request descriptor.", "EXFULL: Exchange full.", "ENOANO: No anode.", "EBADRQC: Invalid request code.", "EBADSLT: Invalid slot.", "EDEADLOCK: Deadlock.", "EBFONT: Bad font file format.", "ENOSTR: Device not a stream.", "ENODATA: No data available.", "ETIME: Timer expired.", "ENOSR: Out of streams resources.", "ENONET: Machine is not on the network.", "ENOPKG: Package not installed.", "EREMOTE: Object is remote.", "ENOLINK: Link has been severed.", "EADV: Advertise error.", "ESRMNT: Srmount error.", "ECOMM: Communication error on send.", "EPROTO: Protocol error.", "EMULTIHOP: Multihop attempted.", "EDOTDOT: RFS specific error.", "EBADMSG: Not a data message.", "EOVERFLOW: Value too large for defined data type.", "ENOTUNIQ: Name not unique on network.", "EBADFD: File descriptor in bad state.", "EREMCHG: Remote address changed.", "ELIBACC: Can not access a needed shared library.", "ELIBBAD: Accessing a corrupted shared library.", "ELIBSCN: .lib section in a.out corrupted.", "ELIBMAX: Attempting to link in too many shared libraries.", "ELIBEXEC: Cannot exec a shared library directly.", "EILSEQ: Illegal byte sequence.", "ERESTART: Interrupted system call should be restarted.", "ESTRPIPE: Streams pipe error.", "EUSERS: Too many users.", "ENOTSOCK: Socket operation on non-socket.", "EDESTADDRREQ: Destination address required.", "EMSGSIZE: Message too long.", "EPROTOTYPE: Protocol wrong type for socket.", "ENOPROTOOPT: Protocol not available.", "EPROTONOSUPPORT: Protocol not supported.", "ESOCKTNOSUPPORT: Socket type not supported.", "EOPNOTSUPP: Operation not supported on transport endpoint.", "EPFNOSUPPORT: Protocol family not supported.", "EAFNOSUPPORT: Address family not supported by protocol.", "EADDRINUSE: Address already in use.", "EADDRNOTAVAIL: Cannot assign requested address.", "ENETDOWN: Network is down.", "ENETUNREACH: Network is unreachable.", "ENETRESET: Network dropped connection because of reset.", "ECONNABORTED: Software caused connection abort.", "ECONNRESET: Connection reset by peer.", "ENOBUFS: No buffer space available.", "EISCONN: Transport endpoint is already connected.", "ENOTCONN: Transport endpoint is not connected.", "ESHUTDOWN: Cannot send after transport endpoint shutdown.", "ETOOMANYREFS: Too many references: cannot splice.", "ETIMEDOUT: Connection timed out.", "ECONNREFUSED: Connection refused.", "EHOSTDOWN: Host is down.", "EHOSTUNREACH: No route to host.", "EALREADY: Operation already in progress.", "EINPROGRESS: Operation now in progress.", "ESTALE: Stale NFS file handle.", "EUCLEAN: Structure needs cleaning.", "ENOTNAM: Not a XENIX named type file.", "ENAVAIL: No XENIX semaphores available.", "EISNAM: Is a named type file.", "EREMOTEIO: Remote I/O error.", "EDQUOT: Quota exceeded.", "ENOMEDIUM: No medium found.", "EMEDIUMTYPE: Wrong medium type.", "ECANCELED: Operation Canceled.", "ENOKEY: Required key not available.", "EKEYEXPIRED: Key has expired.", "EKEYREVOKED: Key has been revoked.", "EKEYREJECTED: Key was rejected by service.", "EOWNERDEAD: Owner died.", "ENOTRECOVERABLE: State not recoverable.", "ERFKILL: Operation not possible due to RF-kill."};
#endif

static char unknown[64];

//--page-split-- error_string

const char *error_string(int error_code) {
  #ifdef UNIX
    if (error_code >= 1 && error_code <= sizeof(strings) / sizeof(char *)) {
      return strings[error_code - 1];
    } else {
      sprintf(unknown, "%d: Unknown error.", error_code);
      return unknown;
    };
  #endif
  #ifdef WINDOWS

    switch (error_code) {
      case 6: return "WSA_INVALID_HANDLE: Specified event object handle is invalid."; break;
      case 8: return "WSA_NOT_ENOUGH_MEMORY: Insufficient memory available."; break;
      case 87: return "WSA_INVALID_PARAMETER: One or more parameters are invalid."; break;
      case 995: return "WSA_OPERATION_ABORTED: Overlapped operation aborted."; break;
      case 996: return "WSA_IO_INCOMPLETE: Overlapped I/O event object not in signaled state."; break;
      case 997: return "WSA_IO_PENDING: Overlapped operations will complete later."; break;
      case 10004: return "WSAEINTR: Interrupted function call."; break;
      case 10009: return "WSAEBADF: File handle is not valid."; break;
      case 10013: return "WSAEACCES: Permission denied."; break;
      case 10014: return "WSAEFAULT: Bad address."; break;
      case 10022: return "WSAEINVAL: Invalid argument."; break;
      case 10024: return "WSAEMFILE: Too many open files."; break;
      case 10035: return "WSAEWOULDBLOCK: Resource temporarily unavailable."; break;
      case 10036: return "WSAEINPROGRESS: Operation now in progress."; break;
      case 10037: return "WSAEALREADY: Operation already in progress."; break;
      case 10038: return "WSAENOTSOCK: Socket operation on nonsocket."; break;
      case 10039: return "WSAEDESTADDRREQ: Destination address required."; break;
      case 10040: return "WSAEMSGSIZE: Message too long."; break;
      case 10041: return "WSAEPROTOTYPE: Protocol wrong type for socket."; break;
      case 10042: return "WSAENOPROTOOPT: Bad protocol option."; break;
      case 10043: return "WSAEPROTONOSUPPORT: Protocol not supported."; break;
      case 10044: return "WSAESOCKTNOSUPPORT: Socket type not supported."; break;
      case 10045: return "WSAEOPNOTSUPP: Operation not supported."; break;
      case 10046: return "WSAEPFNOSUPPORT: Protocol family not supported."; break;
      case 10047: return "WSAEAFNOSUPPORT: Address family not supported by protocol family."; break;
      case 10048: return "WSAEADDRINUSE: Address already in use."; break;
      case 10049: return "WSAEADDRNOTAVAIL: Cannot assign requested address."; break;
      case 10050: return "WSAENETDOWN: Network is down."; break;
      case 10051: return "WSAENETUNREACH: Network is unreachable."; break;
      case 10052: return "WSAENETRESET: Network dropped connection on reset."; break;
      case 10053: return "WSAECONNABORTED: Software caused connection abort."; break;
      case 10054: return "WSAECONNRESET: Connection reset by peer."; break;
      case 10055: return "WSAENOBUFS: No buffer space available."; break;
      case 10056: return "WSAEISCONN: Socket is already connected."; break;
      case 10057: return "WSAENOTCONN: Socket is not connected."; break;
      case 10058: return "WSAESHUTDOWN: Cannot send after socket shutdown."; break;
      case 10059: return "WSAETOOMANYREFS: Too many references."; break;
      case 10060: return "WSAETIMEDOUT: Connection timed out."; break;
      case 10061: return "WSAECONNREFUSED: Connection refused."; break;
      case 10062: return "WSAELOOP: Cannot translate name."; break;
      case 10063: return "WSAENAMETOOLONG: Name too long."; break;
      case 10064: return "WSAEHOSTDOWN: Host is down."; break;
      case 10065: return "WSAEHOSTUNREACH: No route to host."; break;
      case 10066: return "WSAENOTEMPTY: Directory not empty."; break;
      case 10067: return "WSAEPROCLIM: Too many processes."; break;
      case 10068: return "WSAEUSERS: User quota exceeded."; break;
      case 10069: return "WSAEDQUOT: Disk quota exceeded."; break;
      case 10070: return "WSAESTALE: Stale file handle reference."; break;
      case 10071: return "WSAEREMOTE: Item is remote."; break;
      case 10091: return "WSASYSNOTREADY: Network subsystem is unavailable."; break;
      case 10092: return "WSAVERNOTSUPPORTED: Winsock.dll version out of range."; break;
      case 10093: return "WSANOTINITIALISED: Successful WSAStartup not yet performed."; break;
      case 10101: return "WSAEDISCON: Graceful shutdown in progress."; break;
      case 10102: return "WSAENOMORE: No more results."; break;
      case 10103: return "WSAECANCELLED: Call has been canceled."; break;
      case 10104: return "WSAEINVALIDPROCTABLE: Procedure call table is invalid."; break;
      case 10105: return "WSAEINVALIDPROVIDER: Service provider is invalid."; break;
      case 10106: return "WSAEPROVIDERFAILEDINIT: Service provider failed to initialize."; break;
      case 10107: return "WSASYSCALLFAILURE: System call failure."; break;
      case 10108: return "WSASERVICE_NOT_FOUND: Service not found."; break;
      case 10109: return "WSATYPE_NOT_FOUND: Class type not found."; break;
      case 10110: return "WSA_E_NO_MORE: No more results."; break;
      case 10111: return "WSA_E_CANCELLED: Call was canceled."; break;
      case 10112: return "WSAEREFUSED: Database query was refused."; break;
      case 11001: return "WSAHOST_NOT_FOUND: Host not found."; break;
      case 11002: return "WSATRY_AGAIN: Nonauthoritative host not found."; break;
      case 11003: return "WSANO_RECOVERY: This is a nonrecoverable error."; break;
      case 11004: return "WSANO_DATA: Valid name, no data record of requested type."; break;

      default:;

        // http://msdn.microsoft.com/en-us/library/ms680582(v=vs.85).aspx

        // Retrieve the system error message for the last-error code

        LPVOID lpMsgBuf;
        LPVOID lpDisplayBuf;
        DWORD dw = GetLastError();

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );

        if (lpMsgBuf != NULL) {
          // Strip CRLF from end of message.
          char message[strlen(lpMsgBuf) + 1];
          char *i = lpMsgBuf, *o = message;
          for (i = lpMsgBuf; *i != 0; i++) {
            if (*i < 32) continue;
            *o = *i; o++;
          };
          *o = 0;
          snprintf(unknown, 64, "Error %d: %s.", error_code, message);
        } else {
          snprintf(unknown, 64, "Error %d: Whatever that means...", error_code);
        };

        LocalFree(lpMsgBuf);

      return unknown;

    };

  #endif
};
