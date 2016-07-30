#! /bin/sh
# $Id: loader.sh,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $

trap end TERM INT QUIT

end ()
{
  PROCLIST=`ps auxgwwe | grep MPC_LAUN | grep -v "grep MPC_LAUN" | awk '{ print $2; }'` 
  if test ! -z "$PROCLIST"
  then
    echo -n terminating all processes:
    kill -TERM "$PROCLIST"
    echo -n ' .'; sleep 1; echo -n '.'; sleep 1; echo -n '.'; sleep 1; echo -n '.'; sleep 1; echo '.'
    PROCLIST=`ps auxgwwe | grep MPC_LAUN | grep -v "grep MPC_LAUN" | awk '{ print $2; }'` 
    if test ! -z "$PROCLIST"
    then
      echo -n killing all processes:
      kill -KILL "$PROCLIST"
      echo -n ' .'; sleep 1; echo -n '.'; echo -n '.'; echo -n '.'; sleep 1; echo '.'
    fi
  fi
  echo end.
  exit 0
}

while true
do
  read UID
  if test -z "$UID"
  then
    echo EOF
    end
  fi

  IFS="" read MAINCLASS
  if test -z "$MAINCLASS"
  then
    echo EOF
    end
  fi

  IFS="" read CMD
  if test -z "$CMD"
  then
    echo EOF
    end
  fi

  USER=`pw usershow $UID | cut -f1 -d:`
  if test -z "$USER"
  then
    continue
  fi

  echo "Launching: su -m $USER -c \"($CMD)\" &"
  MAINCLASS="$MAINCLASS" MPC_LAUNCHED=MARKED su -m "$USER" -c "($CMD) &"
done > /tmp/loader.log 2>&1

