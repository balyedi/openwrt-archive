#!/usr/bin/haserl -u
<? 
. /usr/lib/webif/webif.sh
header "System" "Firmware upgrade" "Firmware Upgrade"

strip_cybertan() {
	dd if="$FORM_firmware" of=/tmp/upgrade.bin bs=32 skip=1 2>/dev/null
	rm $FORM_firmware
}

[ -z "$FORM_submit" -o -z "$FORM_firmware" ] || {
	[ -n $FORM_firmware ] && {
		HEADER=$(head -c4 $FORM_firmware | hexdump -e "8/1 \"%x\"")
		grep BCM947 /proc/cpuinfo > /dev/null && {
			case "$HEADER" in
				48445230) # TRX
					echo "Firmware is in TRX format<br />"
					mv $FORM_firmware /tmp/upgrade.bin
					UPGRADE=1
				;;
				57353447|57353453) # WRT54G(S)
					echo "Firmware is in Cybertan BIN format, converting... "
					strip_cybertan
					echo "done <br />"
					UPGRADE=1
				;;
				*)
					rm $FORM_firmware
					ERROR="<h2>Error: Invalid firmware file format</h2>"
				;;
			esac
		}
	} || {
		ERROR="<h2>Error: Couldn't open firmware file</h2>"
	}
}
?>
<?if [ -z "$UPGRADE" ] ?>
	<form method="POST" name="upgrade" action="<? echo -n $SCRIPT_NAME ?>" enctype="multipart/form-data">
	<table style="width: 90%; text-align: left;" border="0" cellpadding="2" cellspacing="2" align="center">
	<tbody>
		<tr>
			<td>Options:</td>
			<td>
				<input type="checkbox" name="erase_fs" value="1" checked="checked" />Erase JFFS2 partition<br />
				<input type="checkbox" name="erase_nvram" value="1" />Erase NVRAM
			</td>
		</tr>
		<tr>
			<td>Firmware image to upload:</td>
			<td>
				<input type="file" name="firmware" />
			</td>
		</tr>
			<td />
			<td><input type="submit" name="submit" value="Upgrade" /></td>
		</tr>
	</tbody>
	</table>
	</form>
<?el?>
<?
	ERASE=""
	[ "$FORM_erase_fs" = 1 ] && ERASE="-e linux "
	[ "$FORM_erase_nvram" = 1 ] && ERASE="$ERASE -e nvram "
?>
Upgrading... 
<? mtd -r $ERASE write /tmp/upgrade.bin linux 2>&- >&- ?>
done <br />
<?fi?>

<? footer ?>
<!--
##WEBIF:name:System:3:Firmware upgrade
-->
