#include "Common.h"

namespace Testing
{		
    TEST_MODULE_INITIALIZE( ModuleInit )
    {
        InitializeOnce();
    }

    TEST_CLASS( Generic )
    {
    public:
        TEST_METHOD_INITIALIZE( ClassInitialize )
        {
            _proc.Attach( GetCurrentProcessId() );
        }

        TEST_METHOD( PEB )
        {
            _PEB32 peb32 = { };
            _PEB64 peb64 = { };

            auto ppeb32 = _proc.core().peb32( &peb32 );
            auto ppeb64 = _proc.core().peb64( &peb64 );

            AssertEx::IsNotZero( ppeb64 );
            AssertEx::IsNotZero( peb64.Ldr );

            if (_proc.barrier().targetWow64)
            {
                AssertEx::IsNotZero( ppeb32 );
                AssertEx::IsNotZero( peb32.Ldr );
            }
        }

        TEST_METHOD( TEB )
        {
            _TEB32 teb32 = { };
            _TEB64 teb64 = { };

            auto pteb32 = _proc.threads().getMain()->teb( &teb32 );
            auto pteb64 = _proc.threads().getMain()->teb( &teb64 );

            AssertEx::IsNotZero( pteb64 );
            AssertEx::IsNotZero( teb64.ClientId.UniqueThread );

            if (_proc.barrier().targetWow64)
            {
                AssertEx::IsNotZero( pteb32 );
                AssertEx::IsNotZero( teb32.ClientId.UniqueThread );
                AssertEx::IsNotZero( teb32.ClientId.UniqueThread );
            }
        }

        static DWORD CALLBACK VoidFn( void* )
        {
            return 0;
        }

        TEST_METHOD( InvalidHandles )
        {
            HANDLE hProc = OpenProcess( PROCESS_ALL_ACCESS & ~PROCESS_CREATE_THREAD, FALSE, GetCurrentProcessId() );
            Process proc1( hProc );

            try
            {
                auto thread = proc1.threads().CreateNew( reinterpret_cast<ptr_t>(&VoidFn), 0 );
                AssertEx::Fail( L"Must throw" );
            }
            catch (const nt_exception& e)
            {
                AssertEx::AreEqual( STATUS_ACCESS_DENIED, e.status() );
            }
            

            hProc = OpenProcess( PROCESS_ALL_ACCESS & ~PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
            Process proc2( hProc );

            PEB_T peb = { };
            AssertEx::IsNotZero( proc2.core().peb( &peb ) );
            AssertEx::IsZero( peb.ImageBaseAddress );
            AssertEx::AreEqual( STATUS_ACCESS_DENIED, LastNtStatus() );
        }

    private:
        Process _proc;
    };
}