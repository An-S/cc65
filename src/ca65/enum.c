/*****************************************************************************/
/*                                                                           */
/*                                  enum.c                                   */
/*                                                                           */
/*                               .ENUM command                               */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* (C) 2003      Ullrich von Bassewitz                                       */
/*               R�merstra�e 52                                              */
/*               D-70794 Filderstadt                                         */
/* EMail:        uz@cc65.org                                                 */
/*                                                                           */
/*                                                                           */
/* This software is provided 'as-is', without any expressed or implied       */
/* warranty.  In no event will the authors be held liable for any damages    */
/* arising from the use of this software.                                    */
/*                                                                           */
/* Permission is granted to anyone to use this software for any purpose,     */
/* including commercial applications, and to alter it and redistribute it    */
/* freely, subject to the following restrictions:                            */
/*                                                                           */
/* 1. The origin of this software must not be misrepresented; you must not   */
/*    claim that you wrote the original software. If you use this software   */
/*    in a product, an acknowledgment in the product documentation would be  */
/*    appreciated but is not required.                                       */
/* 2. Altered source versions must be plainly marked as such, and must not   */
/*    be misrepresented as being the original software.                      */
/* 3. This notice may not be removed or altered from any source              */
/*    distribution.                                                          */
/*                                                                           */
/*****************************************************************************/



/* common */
#include "addrsize.h"

/* ca65 */
#include "condasm.h"
#include "enum.h"
#include "error.h"
#include "expr.h"
#include "nexttok.h"
#include "scanner.h"
#include "symbol.h"
#include "symtab.h"



/*****************************************************************************/
/*     	       	      		     Code				     */
/*****************************************************************************/



void DoEnum (void)
/* Handle the .ENUM command */
{
    /* Start at zero */
    long      Offs     = 0;
    ExprNode* BaseExpr = GenLiteralExpr (0);

    /* Check for a name */
    int Anon = (Tok != TOK_IDENT);
    if (!Anon) {
        /* Enter a new scope, then skip the name */
        SymEnterLevel (SVal, ST_ENUM, ADDR_SIZE_ABS);
        NextTok ();
    }

    /* Test for end of line */
    ConsumeSep ();

    /* Read until end of struct */
    while (Tok != TOK_ENDENUM && Tok != TOK_EOF) {

        SymEntry* Sym;
        ExprNode* EnumExpr;

        /* Skip empty lines */
        if (Tok == TOK_SEP) {
            NextTok ();
            continue;
        }

        /* The format is "identifier [ = value ]" */
        if (Tok != TOK_IDENT) {
            /* Maybe it's a conditional? */
            if (!CheckConditionals ()) {
                ErrorSkip ("Identifier expected");
            }
            continue;
        }

        /* We have an identifier, generate a symbol */
        Sym = SymFind (CurrentScope, SVal, SYM_ALLOC_NEW);

        /* Skip the member name */
        NextTok ();

        /* Check for an assignment */
        if (Tok == TOK_EQ) {

            /* Skip the equal sign */
            NextTok ();

            /* Read the new expression */
            EnumExpr = Expression ();

            /* Reset the base expression and the offset */
            FreeExpr (BaseExpr);
            BaseExpr = CloneExpr (EnumExpr);
            Offs     = 0;

        } else {

            /* No assignment, use last value + 1 */
            EnumExpr = GenAddExpr (CloneExpr (BaseExpr), GenLiteralExpr (Offs));

        }

        /* Assign the value to the enum member */
        SymDef (Sym, EnumExpr, ADDR_SIZE_DEFAULT, SF_NONE);

        /* Increment the offset for the next member */
        ++Offs;

        /* Expect end of line */
        ConsumeSep ();
    }

    /* If this is not an anon enum, leave its scope */
    if (!Anon) {
        /* Close the enum scope */
        SymLeaveLevel ();
    }

    /* End of enum definition */
    Consume (TOK_ENDENUM, "`.ENDENUM' expected");

    /* Free the base expression */
    FreeExpr (BaseExpr);
}


